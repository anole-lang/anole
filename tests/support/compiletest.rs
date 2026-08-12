use std::ffi::OsStr;
use std::fmt::Write as _;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;
use std::sync::atomic::{AtomicU64, Ordering};
use std::time::{SystemTime, UNIX_EPOCH};

use anole::{Interpreter, Lexer, Location, Parser, TokenKind};

const ROOTS: &[&str] = &["tests/compile", "example"];
static TEMP_ID: AtomicU64 = AtomicU64::new(0);

pub(crate) fn run(bless: bool, filter: Option<&str>) {
    let mut cases = Vec::new();
    for root in ROOTS {
        discover(Path::new(root), &mut cases);
    }
    cases.sort();
    if let Some(filter) = filter {
        cases.retain(|case| case.to_string_lossy().contains(filter));
    }
    assert!(
        !cases.is_empty(),
        "no compile tests found{}",
        filter.map_or(String::new(), |filter| format!(" for filter {filter:?}"))
    );
    println!("running {} compile tests", cases.len());

    let mut failures = Vec::new();
    for source_path in &cases {
        if let Err(failure) = run_case(source_path, bless) {
            failures.push(failure);
        }
    }
    if !failures.is_empty() {
        panic!(
            "{} of {} compile tests failed:\n\n{}",
            failures.len(),
            cases.len(),
            failures.join("\n\n")
        );
    }
    println!("test result: ok. {} passed; 0 failed", cases.len());
}

fn discover(directory: &Path, cases: &mut Vec<PathBuf>) {
    let Ok(entries) = fs::read_dir(directory) else {
        return;
    };
    for entry in entries {
        let entry =
            entry.unwrap_or_else(|error| panic!("failed to read {}: {error}", directory.display()));
        let path = entry.path();
        if path.is_dir() {
            discover(&path, cases);
        } else if path.extension() == Some(OsStr::new("anole"))
            && sidecar(&path, "result").is_file()
        {
            cases.push(path);
        }
    }
}

fn run_case(source_path: &Path, bless: bool) -> Result<(), String> {
    let result_path = sidecar(source_path, "result");
    let expected_result = fs::read_to_string(&result_path)
        .map_err(|error| format!("{}: {error}", result_path.display()))?;
    let directives = Directives::parse(&expected_result)
        .map_err(|error| format!("{}: {error}", result_path.display()))?;
    let temporary = TemporaryDirectory::copy_of(
        source_path
            .parent()
            .expect("compile test source must have a parent"),
    )?;
    let copied_source = temporary.path.join(
        source_path
            .file_name()
            .expect("compile test source must have a file name"),
    );
    let mut source =
        fs::read(&copied_source).map_err(|error| format!("{}: {error}", source_path.display()))?;
    if directives.no_final_newline && source.last() == Some(&b'\n') {
        source.pop();
    }
    let mut actual = execute(&directives, &copied_source, &source)?;
    actual.normalize_path(&temporary.path);
    let actual_result = directives.render_result(&actual.result);

    let mut mismatches = Vec::new();
    compare_or_bless(
        &result_path,
        expected_result.as_bytes(),
        actual_result.as_bytes(),
        bless,
        false,
        &mut mismatches,
    )?;
    compare_or_bless(
        &sidecar(source_path, "stdout"),
        &read_optional(&sidecar(source_path, "stdout"))?,
        &actual.stdout,
        bless,
        true,
        &mut mismatches,
    )?;
    compare_or_bless(
        &sidecar(source_path, "stderr"),
        &read_optional(&sidecar(source_path, "stderr"))?,
        &actual.stderr,
        bless,
        true,
        &mut mismatches,
    )?;

    if mismatches.is_empty() {
        Ok(())
    } else {
        Err(format!(
            "{}\n{}\n\nTo update the expected output, run:\n    cargo test --test compile_tests -- --bless",
            source_path.display(),
            mismatches.join("\n")
        ))
    }
}

#[derive(Clone, Copy)]
enum Mode {
    Run,
    Parse,
    Lex,
    Cli,
}

struct Directives {
    mode: Mode,
    options: Vec<String>,
    arguments: Vec<String>,
    no_final_newline: bool,
}

impl Directives {
    fn parse(result: &str) -> Result<Self, String> {
        let mut mode = None;
        let mut options = Vec::new();
        let mut arguments = Vec::new();
        let mut no_final_newline = false;
        for (index, raw_line) in result.lines().enumerate() {
            let line = raw_line.trim();
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            let Some((key, value)) = line.split_once(':') else {
                return Err(format!("line {}: expected `key: value`", index + 1));
            };
            let key = key.trim();
            let value = value.trim();
            match key {
                "mode" => {
                    mode = Some(match value {
                        "run" => Mode::Run,
                        "parse" => Mode::Parse,
                        "lex" => Mode::Lex,
                        "cli" => Mode::Cli,
                        _ => return Err(format!("line {}: unknown mode {value:?}", index + 1)),
                    });
                }
                "option" => options.push(value.to_owned()),
                "arg" => arguments.push(value.to_owned()),
                "no-final-newline" => {
                    no_final_newline = match value {
                        "true" => true,
                        "false" => false,
                        _ => {
                            return Err(format!("line {}: expected `true` or `false`", index + 1));
                        }
                    };
                }
                "status" | "message" | "line" | "column" => {}
                _ => return Err(format!("line {}: unknown key {key:?}", index + 1)),
            }
        }
        Ok(Self {
            mode: mode.ok_or_else(|| "missing `mode`".to_owned())?,
            options,
            arguments,
            no_final_newline,
        })
    }

    fn render_result(&self, outcome: &Outcome) -> String {
        let mut result = format!("mode: {}\n", self.mode_name());
        for option in &self.options {
            writeln!(result, "option: {option}").unwrap();
        }
        for argument in &self.arguments {
            writeln!(result, "arg: {argument}").unwrap();
        }
        if self.no_final_newline {
            result.push_str("no-final-newline: true\n");
        }
        match outcome {
            Outcome::Success => result.push_str("status: success\n"),
            Outcome::Exit(code) => writeln!(result, "status: {code}").unwrap(),
            Outcome::Failure { message, location } => {
                result.push_str("status: failure\n");
                writeln!(result, "message: {message}").unwrap();
                if let Some(location) = location {
                    writeln!(result, "line: {}", location.line).unwrap();
                    writeln!(result, "column: {}", location.column).unwrap();
                }
            }
        }
        result
    }

    fn mode_name(&self) -> &'static str {
        match self.mode {
            Mode::Run => "run",
            Mode::Parse => "parse",
            Mode::Lex => "lex",
            Mode::Cli => "cli",
        }
    }
}

struct Actual {
    result: Outcome,
    stdout: Vec<u8>,
    stderr: Vec<u8>,
}

impl Actual {
    fn normalize_path(&mut self, path: &Path) {
        let displayed = path.to_string_lossy();
        self.stdout = replace_bytes(&self.stdout, displayed.as_bytes(), b"$TEST_DIR");
        self.stderr = replace_bytes(&self.stderr, displayed.as_bytes(), b"$TEST_DIR");
        if let Outcome::Failure { message, .. } = &mut self.result {
            *message = message.replace(displayed.as_ref(), "$TEST_DIR");
        }
    }
}

enum Outcome {
    Success,
    Exit(i32),
    Failure {
        message: String,
        location: Option<Location>,
    },
}

fn execute(directives: &Directives, path: &Path, source: &[u8]) -> Result<Actual, String> {
    match directives.mode {
        Mode::Run => {
            let mut interpreter = Interpreter::with_arguments(directives.arguments.clone());
            match interpreter.run_file_bytes(source, path) {
                Ok(stdout) => Ok(Actual {
                    result: Outcome::Success,
                    stdout: stdout.into_bytes(),
                    stderr: Vec::new(),
                }),
                Err(error) => Ok(Actual {
                    result: Outcome::Failure {
                        message: error.message,
                        location: error.location,
                    },
                    stdout: Vec::new(),
                    stderr: Vec::new(),
                }),
            }
        }
        Mode::Parse => match Parser::new_bytes(source, display_name(path)) {
            Ok(parser) => match parser.parse() {
                Ok(program) => Ok(Actual {
                    result: Outcome::Success,
                    stdout: format!("{program:#?}\n").into_bytes(),
                    stderr: Vec::new(),
                }),
                Err(error) => Ok(Actual {
                    result: Outcome::Failure {
                        message: error.message,
                        location: Some(error.location),
                    },
                    stdout: Vec::new(),
                    stderr: Vec::new(),
                }),
            },
            Err(error) => Ok(Actual {
                result: Outcome::Failure {
                    message: error.message,
                    location: Some(error.location),
                },
                stdout: Vec::new(),
                stderr: Vec::new(),
            }),
        },
        Mode::Lex => match Lexer::new_bytes(source, display_name(path)).tokenize() {
            Ok(tokens) => {
                let mut stdout = String::new();
                for token in tokens {
                    writeln!(
                        stdout,
                        "{:?} {}:{} {:?}",
                        token.kind, token.location.line, token.location.column, token.lexeme
                    )
                    .unwrap();
                    if token.kind == TokenKind::End {
                        break;
                    }
                }
                Ok(Actual {
                    result: Outcome::Success,
                    stdout: stdout.into_bytes(),
                    stderr: Vec::new(),
                })
            }
            Err(error) => Ok(Actual {
                result: Outcome::Failure {
                    message: error.message,
                    location: Some(error.location),
                },
                stdout: Vec::new(),
                stderr: Vec::new(),
            }),
        },
        Mode::Cli => {
            let parent = path
                .parent()
                .expect("copied test source must have a parent");
            let mut command = Command::new(env!("CARGO_BIN_EXE_anole"));
            command.current_dir(parent);
            command.args(&directives.options);
            command.arg(path.file_name().expect("test source must have a file name"));
            command.args(&directives.arguments);
            let output = command
                .output()
                .map_err(|error| format!("failed to run {}: {error}", path.display()))?;
            Ok(Actual {
                result: Outcome::Exit(output.status.code().unwrap_or(-1)),
                stdout: output.stdout,
                stderr: output.stderr,
            })
        }
    }
}

fn compare_or_bless(
    path: &Path,
    expected: &[u8],
    actual: &[u8],
    bless: bool,
    remove_empty: bool,
    mismatches: &mut Vec<String>,
) -> Result<(), String> {
    if expected == actual {
        return Ok(());
    }
    if bless {
        if remove_empty && actual.is_empty() {
            if path.exists() {
                fs::remove_file(path)
                    .map_err(|error| format!("failed to remove {}: {error}", path.display()))?;
            }
        } else {
            fs::write(path, actual)
                .map_err(|error| format!("failed to update {}: {error}", path.display()))?;
        }
        return Ok(());
    }
    mismatches.push(diff(path, expected, actual));
    Ok(())
}

fn diff(path: &Path, expected: &[u8], actual: &[u8]) -> String {
    let expected = String::from_utf8_lossy(expected);
    let actual = String::from_utf8_lossy(actual);
    let first_difference = expected
        .lines()
        .zip(actual.lines())
        .position(|(expected, actual)| expected != actual)
        .map_or_else(
            || expected.lines().count().min(actual.lines().count()) + 1,
            |index| index + 1,
        );
    format!(
        "{} differs at line {first_difference}\n--- expected\n{expected}\n+++ actual\n{actual}",
        path.display()
    )
}

fn display_name(path: &Path) -> String {
    path.file_name()
        .unwrap_or(path.as_os_str())
        .to_string_lossy()
        .into_owned()
}

fn sidecar(source: &Path, extension: &str) -> PathBuf {
    let mut path = source.as_os_str().to_owned();
    path.push(format!(".{extension}"));
    PathBuf::from(path)
}

fn read_optional(path: &Path) -> Result<Vec<u8>, String> {
    match fs::read(path) {
        Ok(contents) => Ok(contents),
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => Ok(Vec::new()),
        Err(error) => Err(format!("{}: {error}", path.display())),
    }
}

fn replace_bytes(contents: &[u8], from: &[u8], to: &[u8]) -> Vec<u8> {
    if from.is_empty() {
        return contents.to_vec();
    }
    let mut replaced = Vec::with_capacity(contents.len());
    let mut offset = 0;
    while let Some(relative) = contents[offset..]
        .windows(from.len())
        .position(|window| window == from)
    {
        let position = offset + relative;
        replaced.extend_from_slice(&contents[offset..position]);
        replaced.extend_from_slice(to);
        offset = position + from.len();
    }
    replaced.extend_from_slice(&contents[offset..]);
    replaced
}

struct TemporaryDirectory {
    path: PathBuf,
}

impl TemporaryDirectory {
    fn copy_of(source: &Path) -> Result<Self, String> {
        let unique = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map_err(|error| error.to_string())?
            .as_nanos();
        let id = TEMP_ID.fetch_add(1, Ordering::Relaxed);
        let path = std::env::temp_dir().join(format!(
            "anole-compiletest-{}-{unique}-{id}",
            std::process::id()
        ));
        fs::create_dir(&path)
            .map_err(|error| format!("failed to create {}: {error}", path.display()))?;
        copy_directory(source, &path)?;
        Ok(Self { path })
    }
}

impl Drop for TemporaryDirectory {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.path);
    }
}

fn copy_directory(source: &Path, destination: &Path) -> Result<(), String> {
    for entry in fs::read_dir(source)
        .map_err(|error| format!("failed to read {}: {error}", source.display()))?
    {
        let entry = entry.map_err(|error| error.to_string())?;
        let source_path = entry.path();
        let destination_path = destination.join(entry.file_name());
        if source_path.is_dir() {
            fs::create_dir(&destination_path).map_err(|error| {
                format!("failed to create {}: {error}", destination_path.display())
            })?;
            copy_directory(&source_path, &destination_path)?;
        } else if matches!(
            source_path.extension().and_then(OsStr::to_str),
            Some("ir" | "rd" | "result" | "stdout" | "stderr")
        ) {
            continue;
        } else {
            fs::copy(&source_path, &destination_path).map_err(|error| {
                format!(
                    "failed to copy {} to {}: {error}",
                    source_path.display(),
                    destination_path.display()
                )
            })?;
        }
    }
    Ok(())
}
