use std::fs;
use std::io::{self, IsTerminal, Write};
use std::path::{Component, Path, PathBuf};

#[cfg(unix)]
use std::os::unix::ffi::OsStrExt;

use anole::{Interpreter, VERSION_LITERAL};

fn main() {
    if let Err(message) = run() {
        eprintln!("{message}");
    }
}

fn run() -> Result<(), String> {
    let arguments: Vec<_> = std::env::args_os().skip(1).collect();
    // The first argument ending in `.anole` separates interpreter options from
    // program arguments. Directory entry points have no such boundary, so all
    // arguments remain in the interpreter prefix.
    let anole_boundary = arguments
        .iter()
        .position(|argument| argument.to_string_lossy().ends_with(".anole"));
    let interpreter_end = anole_boundary.map_or(arguments.len(), |index| index + 1);
    let interpreter_arguments = &arguments[..interpreter_end];
    if interpreter_arguments
        .iter()
        .any(|argument| argument == "--version" || argument == "-version")
    {
        println!("Anole {VERSION_LITERAL}");
        return Ok(());
    }

    let file_index = interpreter_arguments
        .iter()
        .position(|argument| !argument.to_string_lossy().starts_with('-'));
    if let Some(file_index) = file_index {
        let file = &arguments[file_index];
        let mut path = PathBuf::from(file);
        if !path_extension_is(&path, "anole") {
            path.push("__init__.anole");
        }
        if path.is_relative() {
            path = std::env::current_dir()
                .map_err(|error| error.to_string())?
                .join(path);
        }
        path = lexically_normal(&path);
        let source = if path.is_dir() {
            fs::File::open(&path).map(|_| Vec::new())
        } else {
            fs::read(&path)
        };
        let source = if let Ok(source) = source {
            source
        } else {
            let mut stderr = io::stderr().lock();
            stderr
                .write_all(b"\x1b[1mTraceback:\n\x1b[0m\x1b[31merror: \x1b[0mcannot open file ")
                .map_err(|error| error.to_string())?;
            stderr
                .write_all(&path_as_bytes(&path))
                .map_err(|error| error.to_string())?;
            stderr.write_all(b"\n").map_err(|error| error.to_string())?;
            return Ok(());
        };
        let script_arguments = anole_boundary
            .map(|boundary| arguments[boundary..].to_vec())
            .unwrap_or_default();
        let mut interpreter = Interpreter::with_os_arguments(script_arguments);
        interpreter.set_stream_output(true);
        if let Err(error) = interpreter.run_file_bytes(&source, &path) {
            let mut stderr = io::stderr().lock();
            stderr
                .write_all(&error.render_bytes())
                .map_err(|error| error.to_string())?;
            stderr.write_all(b"\n").map_err(|error| error.to_string())?;
            return Ok(());
        }
        if !interpreter.is_halted()
            && interpreter_arguments
                .iter()
                .any(|argument| argument == "-r" || argument == "--r")
        {
            let mut rd_path = path.as_os_str().to_os_string();
            rd_path.push(".rd");
            let rd_path = PathBuf::from(rd_path);
            interpreter
                .write_debug_ir(&path, &rd_path)
                .map_err(|error| error.to_string())?;
        }
        return Ok(());
    }

    if !arguments.is_empty() {
        println!("invalid command-line argument(s)");
        return Ok(());
    }

    repl()
}

#[cfg(unix)]
fn path_as_bytes(path: &Path) -> Vec<u8> {
    path.as_os_str().as_bytes().to_vec()
}

#[cfg(not(unix))]
fn path_as_bytes(path: &Path) -> Vec<u8> {
    path.to_string_lossy().as_bytes().to_vec()
}

fn lexically_normal(path: &Path) -> PathBuf {
    if path.as_os_str().is_empty() {
        return PathBuf::new();
    }
    let preserve_trailing_separator = path_preserves_trailing_separator(path);
    let mut normalized = PathBuf::new();
    for component in path.components() {
        match component {
            Component::Prefix(_) | Component::RootDir | Component::Normal(_) => {
                normalized.push(component.as_os_str());
            }
            Component::CurDir => {}
            Component::ParentDir => {
                if matches!(
                    normalized.components().next_back(),
                    Some(Component::Normal(_))
                ) {
                    normalized.pop();
                } else if !normalized.has_root() {
                    normalized.push(component.as_os_str());
                }
            }
        }
    }
    if preserve_trailing_separator
        && matches!(
            normalized.components().next_back(),
            Some(Component::Normal(_))
        )
    {
        normalized.push("");
    }
    if normalized.as_os_str().is_empty() {
        normalized.push(".");
    }
    normalized
}

#[cfg(unix)]
fn path_preserves_trailing_separator(path: &Path) -> bool {
    let bytes = path.as_os_str().as_bytes();
    if path_ends_with_directory_separator(path) {
        return true;
    }
    matches!(
        bytes.rsplit(|byte| *byte == b'/').next(),
        Some(b"." | b"..")
    )
}

#[cfg(not(unix))]
fn path_preserves_trailing_separator(path: &Path) -> bool {
    let path = path.as_os_str().to_string_lossy();
    path_ends_with_directory_separator(Path::new(path.as_ref()))
        || matches!(path.rsplit(['/', '\\']).next(), Some("." | ".."))
}

#[cfg(unix)]
fn path_ends_with_directory_separator(path: &Path) -> bool {
    path.as_os_str().as_bytes().last() == Some(&b'/')
}

#[cfg(not(unix))]
fn path_ends_with_directory_separator(path: &Path) -> bool {
    path.as_os_str().to_string_lossy().ends_with(['/', '\\'])
}

#[cfg(unix)]
fn path_extension_is(path: &Path, expected: &str) -> bool {
    let filename = path
        .as_os_str()
        .as_bytes()
        .rsplit(|byte| *byte == b'/')
        .next()
        .unwrap_or_default();
    if filename.is_empty() || matches!(filename, b"." | b"..") {
        return false;
    }
    filename
        .iter()
        .rposition(|byte| *byte == b'.')
        .is_some_and(|dot| dot != 0 && &filename[dot + 1..] == expected.as_bytes())
}

#[cfg(not(unix))]
fn path_extension_is(path: &Path, expected: &str) -> bool {
    let path = path.as_os_str().to_string_lossy();
    let filename = path.rsplit(['/', '\\']).next().unwrap_or_default();
    if filename.is_empty() || matches!(filename, "." | "..") {
        return false;
    }
    filename
        .rfind('.')
        .is_some_and(|dot| dot != 0 && &filename[dot + 1..] == expected)
}

fn repl() -> Result<(), String> {
    println!(
        "    _                _\n   / \\   _ __   ___ | | ___\n  / _ \\ | '_ \\ / _ \\| |/ _ \\\n / ___ \\| | | | (_) | |  __/   {VERSION_LITERAL}\n/_/   \\_\\_| |_|\\___/|_|\\___|\n"
    );
    let mut interpreter = Interpreter::new();
    interpreter.set_stream_output(true);
    loop {
        let Some(source) = read_repl_source(&interpreter)? else {
            return Ok(());
        };
        if source.trim().is_empty() {
            continue;
        }
        match interpreter.run_repl(&source) {
            Ok(_) => {}
            Err(error) => eprintln!("{error}"),
        }
        if interpreter.is_halted() {
            return Ok(());
        }
    }
}

fn read_repl_source(interpreter: &Interpreter) -> Result<Option<String>, String> {
    let mut source = String::new();
    loop {
        let prompt = if source.is_empty() { ">> " } else { ".. " };
        print!("{prompt}");
        io::stdout().flush().map_err(|error| error.to_string())?;
        let mut line = String::new();
        if io::stdin()
            .read_line(&mut line)
            .map_err(|error| error.to_string())?
            == 0
        {
            return Ok(None);
        }
        if !io::stdin().is_terminal() {
            print!("{line}");
            if !line.ends_with('\n') {
                println!();
            }
            io::stdout().flush().map_err(|error| error.to_string())?;
        }
        source.push_str(&line);
        if interpreter.repl_input_complete(&source) {
            return Ok(Some(source));
        }
    }
}
