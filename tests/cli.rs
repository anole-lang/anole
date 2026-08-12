use std::fs::{self, File, FileTimes};
use std::process::{Command, Stdio};
use std::time::{SystemTime, UNIX_EPOCH};

#[cfg(unix)]
use std::os::unix::fs::PermissionsExt;

#[test]
fn reports_the_legacy_version_literal() {
    for option in ["--version", "-version"] {
        let output = Command::new(env!("CARGO_BIN_EXE_anole"))
            .arg(option)
            .output()
            .unwrap();
        assert!(output.status.success());
        assert_eq!(
            String::from_utf8(output.stdout).unwrap(),
            "Anole 0.0.24 2021/12/12\n",
            "option: {option}",
        );
    }
}

#[test]
fn accepts_the_double_dash_spelling_of_the_read_option() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-double-dash-r-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("main.anole");
    fs::write(&script, "println(42);").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("--r")
        .arg(&script)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"42\n");
    assert!(script.with_extension("anole.rd").is_file());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn directory_entry_points_keep_all_arguments_in_the_cli_prefix() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-directory-cli-{unique}"));
    let package = directory.join("package");
    fs::create_dir_all(&package).unwrap();
    fs::write(
        package.join("__init__.anole"),
        "use env; println(env.args().size());",
    )
    .unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&package)
        .arg("-r")
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"0\n");
    assert!(output.stderr.is_empty());
    assert!(package.join("__init__.anole.rd").is_file());

    let version = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&package)
        .arg("--version")
        .output()
        .unwrap();
    assert_eq!(version.stdout, b"Anole 0.0.24 2021/12/12\n");
    assert!(version.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn anole_suffixed_directories_execute_as_empty_files() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-empty-file-{unique}.anole"));
    fs::create_dir(&directory).unwrap();
    fs::write(directory.join("__init__.anole"), "println(99);").unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&directory)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert!(output.stderr.is_empty());
    let mut ir_path = directory.as_os_str().to_os_string();
    ir_path.push(".ir");
    assert!(std::path::PathBuf::from(ir_path).is_file());
    let _ = fs::remove_file(directory.with_extension("anole.ir"));
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn trailing_separator_hides_an_anole_directory_extension() {
    use std::ffi::OsString;

    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let root = std::env::temp_dir().join(format!("anole-trailing-entry-{unique}"));
    let directory = root.join("package.anole");
    fs::create_dir_all(&directory).unwrap();
    fs::write(directory.join("__init__.anole"), "println(99);").unwrap();
    for suffix in ["/", "/."] {
        let mut entry = OsString::from(directory.as_os_str());
        entry.push(suffix);
        let output = Command::new(env!("CARGO_BIN_EXE_anole"))
            .arg(entry)
            .output()
            .unwrap();
        assert!(output.status.success());
        assert_eq!(output.stdout, b"99\n", "suffix: {suffix}");
        assert!(output.stderr.is_empty(), "suffix: {suffix}");
    }
    assert!(directory.join("__init__.anole.ir").is_file());

    fs::remove_dir_all(root).unwrap();
}

#[cfg(unix)]
#[test]
fn unreadable_anole_suffixed_directories_fail_to_open() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("anole-unreadable-{unique}.anole"));
    fs::create_dir(&path).unwrap();
    fs::set_permissions(&path, fs::Permissions::from_mode(0o0)).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();

    fs::set_permissions(&path, fs::Permissions::from_mode(0o700)).unwrap();
    fs::remove_dir(&path).unwrap();
    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert_eq!(
        output.stderr,
        format!(
            "\u{1b}[1mTraceback:\n\u{1b}[0m\u{1b}[31merror: \u{1b}[0mcannot open file {}\n",
            path.display()
        )
        .as_bytes()
    );
}

#[test]
fn executes_anole_files() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("anole-cli-{unique}.anole"));
    fs::write(&path, "println(6 * 7);").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    fs::remove_file(&path).unwrap();
    fs::remove_file(path.with_extension("anole.ir")).unwrap();
    assert!(output.status.success());
    assert_eq!(String::from_utf8(output.stdout).unwrap(), "42\n");
    assert!(output.stderr.is_empty());
}

#[test]
fn preserves_non_utf8_bytes_inside_source_string_literals() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-source-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("bytes.anole");
    fs::write(&path, b"print(\"\x80\x81\");").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, [0x80, 0x81]);
    assert!(output.stderr.is_empty());

    // The second execution reads the string from the serialized legacy IR.
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("-r")
        .arg(&path)
        .output()
        .unwrap();
    assert!(cached.status.success());
    assert_eq!(cached.stdout, [0x80, 0x81]);
    assert!(cached.stderr.is_empty());
    assert!(
        fs::read(path.with_extension("anole.ir"))
            .unwrap()
            .windows(2)
            .any(|window| window == [0x80, 0x81])
    );
    assert!(
        fs::read(path.with_extension("anole.rd"))
            .unwrap()
            .windows(2)
            .any(|window| window == [0x80, 0x81])
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn keeps_distinct_non_utf8_identifier_bytes() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-identifiers-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("identifiers.anole");
    fs::write(
        &path,
        b"@\x80: 1; @\x81: 2; @mapping: dict {\"\x80\" => 3, \"\x81\" => 4}; @\x82(left, right): left * 10 + right; infixop 50 \x82; println(\x80); println(\x81); println(mapping.\x80); println(mapping.\x81); println(1 \x82 2);",
    )
    .unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();

    assert!(output.status.success());
    assert_eq!(output.stdout, b"1\n2\n3\n4\n12\n");
    assert!(output.stderr.is_empty());

    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(cached.stdout, b"1\n2\n3\n4\n12\n");
    assert!(cached.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn preserves_non_utf8_identifier_bytes_in_runtime_errors() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-identifier-error-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("identifier-error.anole");
    for (source, identifier) in [
        (b"println(\x83);".as_slice(), 0x83),
        (b"use \x84;".as_slice(), 0x84),
        (b"@f(\x85): none; f();".as_slice(), 0x85),
        (b"(1).\x86;".as_slice(), 0x86),
    ] {
        fs::write(&path, source).unwrap();
        let _ = fs::remove_file(path.with_extension("anole.ir"));

        let output = Command::new(env!("CARGO_BIN_EXE_anole"))
            .arg(&path)
            .output()
            .unwrap();

        assert!(output.status.success());
        assert!(output.stdout.is_empty());
        assert_eq!(
            output
                .stderr
                .iter()
                .filter(|byte| **byte == identifier)
                .count(),
            1,
            "the runtime error message preserves identifier byte {identifier:#x}",
        );
        let encoded = [0xee, 0x82, identifier];
        assert!(!output.stderr.windows(3).any(|bytes| bytes == encoded));
    }
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn preserves_raw_source_bytes_in_parser_diagnostics() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-parser-error-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("parser-error.anole");
    fs::write(&path, b"@\x87: 1; println(;").unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();

    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert!(output.stderr.contains(&0x87));
    assert!(
        !output
            .stderr
            .windows(3)
            .any(|bytes| bytes == [0xef, 0xbf, 0xbd])
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn source_byte_ff_stops_lexing_on_signed_char_platforms() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-eof-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("eof.anole");
    fs::write(&path, b"print(\"before\");\xffprint(\"after\");").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"before");
    assert!(output.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn eval_parses_runtime_strings_as_raw_source_bytes() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-byte-eval-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("eval.anole");
    fs::write(&path, b"print(eval(\"\\\"\x80\\\"\"));").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, [0x80]);
    assert!(output.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn missing_entry_files_use_a_runtime_traceback() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("anole-missing-{unique}.anole"));
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert_eq!(
        String::from_utf8(output.stderr).unwrap(),
        format!(
            "\u{1b}[1mTraceback:\n\u{1b}[0m\u{1b}[31merror: \u{1b}[0mcannot open file {}\n",
            path.display()
        )
    );
}

#[test]
fn resolves_a_missing_relative_entry_path_against_the_working_directory() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-relative-missing-{unique}"));
    fs::create_dir(&directory).unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("missing.anole")
        .current_dir(&directory)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert_eq!(
        String::from_utf8(output.stderr).unwrap(),
        format!(
            "\u{1b}[1mTraceback:\n\u{1b}[0m\u{1b}[31merror: \u{1b}[0mcannot open file {}\n",
            directory.join("missing.anole").display()
        )
    );
    fs::remove_dir(directory).unwrap();
}

#[test]
fn lexically_normalizes_entry_paths_before_reporting_errors() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-normalized-entry-{unique}"));
    fs::create_dir_all(directory.join("nested")).unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("nested/../missing.anole")
        .current_dir(&directory)
        .output()
        .unwrap();
    assert_eq!(
        String::from_utf8(output.stderr).unwrap(),
        format!(
            "\u{1b}[1mTraceback:\n\u{1b}[0m\u{1b}[31merror: \u{1b}[0mcannot open file {}\n",
            directory.join("missing.anole").display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn named_module_lookup_does_not_search_a_working_directory_lib_folder() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-module-lookup-{unique}"));
    fs::create_dir_all(directory.join("lib")).unwrap();
    fs::create_dir_all(directory.join("scripts")).unwrap();
    fs::write(directory.join("lib/cwd_only.anole"), "@answer: 42;").unwrap();
    let script = directory.join("scripts/main.anole");
    fs::write(&script, "use cwd_only; println(cwd_only.answer);").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&script)
        .current_dir(&directory)
        .output()
        .unwrap();
    assert!(output.stdout.is_empty());
    assert!(String::from_utf8_lossy(&output.stderr).contains("no module named cwd_only"));
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn writes_expected_ir_for_a_basic_program() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-basic-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("basic.anole");
    fs::write(&path, include_str!("fixtures/ir/basic.anole")).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"42\n");

    let actual = fs::read(path.with_extension("anole.ir")).unwrap();
    let expected = decode_hex(&include_str!("fixtures/ir/basic.ir.hex").replace('\n', ""));
    assert_eq!(actual, expected);
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn executes_a_newer_valid_ir_cache_instead_of_reparsing_source() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-cache-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("cached.anole");
    fs::write(&path, "println(42);").unwrap();

    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(first.stdout, b"42\n");
    assert!(first.stderr.is_empty());

    fs::write(&path, "println(99);").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(cached.status.success());
    assert_eq!(cached.stdout, b"42\n");
    assert!(cached.stderr.is_empty());
}

#[test]
fn ignores_trailing_bytes_in_a_newer_ir_cache() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-trailing-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("cached.anole");
    fs::write(&path, "println(42);").unwrap();
    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(first.stdout, b"42\n");

    let ir_path = path.with_extension("anole.ir");
    let mut ir = fs::read(&ir_path).unwrap();
    ir.extend_from_slice(b"ignored trailing bytes");
    fs::write(&ir_path, ir).unwrap();
    fs::write(&path, "println(99);").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();

    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(cached.stdout, b"42\n");
    assert!(cached.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn reports_an_invalid_constant_tag_in_a_newer_ir_cache() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-tag-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("cached.anole");
    fs::write(&path, "println(99);").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();
    let mut ir = Vec::new();
    for value in [20_210_213_u64, 1, 0, 0] {
        ir.extend_from_slice(&value.to_ne_bytes());
    }
    ir.push(b'x');
    fs::write(path.with_extension("anole.ir"), ir).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.stdout.is_empty());
    assert_eq!(output.stderr, b"WTF, you want me to eat shit?!\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn allows_cached_jumps_past_the_end_of_the_instruction_stream() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-jump-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("cached.anole");
    fs::write(&path, "println(99);").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();
    let mut ir = Vec::new();
    for value in [20_210_213_u64, 0, 1, 0] {
        ir.extend_from_slice(&value.to_ne_bytes());
    }
    ir.push(19); // Opcode::Jump
    ir.extend_from_slice(&999_u64.to_ne_bytes());
    fs::write(path.with_extension("anole.ir"), ir).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.stdout.is_empty());
    assert!(output.stderr.is_empty());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn cached_ir_forces_delayed_values_when_they_are_loaded() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-delay-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("delayed.anole");
    fs::write(
        &path,
        concat!(
            "@answer: delay 6 * 7; println(answer);",
            "class Returned { __init__(self) { return 43; } }",
            "println(Returned());",
        ),
    )
    .unwrap();

    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(first.stdout, b"42\n43\n");
    assert!(first.stderr.is_empty());

    fs::write(&path, "this source must not be parsed;").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(cached.status.success());
    assert_eq!(cached.stdout, b"42\n43\n");
    assert!(cached.stderr.is_empty());
}

#[test]
fn cached_ir_executes_continuations_in_composite_contexts() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-continuation-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("continuation.anole");
    fs::write(
        &path,
        include_str!("fixtures/continuation_vm_contexts.anole"),
    )
    .unwrap();

    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert_eq!(first.stdout, b"42\n43\n44\n");
    assert!(first.stderr.is_empty());

    fs::write(&path, "this stale source must not be parsed;").unwrap();
    File::open(&path)
        .unwrap()
        .set_times(FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)))
        .unwrap();
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(cached.status.success());
    assert_eq!(cached.stdout, first.stdout);
    assert!(cached.stderr.is_empty());
}

#[test]
fn cached_ir_executes_continuations_in_imported_modules() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-cont-module-{unique}"));
    fs::create_dir(&directory).unwrap();
    let main = directory.join("main.anole");
    let dependency = directory.join("dependency.anole");
    fs::write(
        &main,
        concat!(
            "use dependency; println(dependency.answer);",
            "println(dependency.forced); println(dependency.get());",
        ),
    )
    .unwrap();
    fs::write(
        &dependency,
        concat!(
            "@answer: call_with_current_continuation(@(continuation): continuation(42));",
            "@delayed: delay call_with_current_continuation(@(continuation): continuation(43));",
            "@forced: delayed;",
            "@get(): call_with_current_continuation(@(continuation): continuation(44));",
        ),
    )
    .unwrap();

    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(&directory)
        .arg(&main)
        .output()
        .unwrap();
    assert_eq!(first.stdout, b"42\n43\n44\n");
    assert!(first.stderr.is_empty());

    for path in [&main, &dependency] {
        fs::write(path, "this stale source must not be parsed;").unwrap();
        File::open(path)
            .unwrap()
            .set_times(
                FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)),
            )
            .unwrap();
    }
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(&directory)
        .arg(&main)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(cached.status.success());
    assert_eq!(cached.stdout, first.stdout);
    assert!(cached.stderr.is_empty());
}

#[test]
fn executes_newer_ir_caches_across_language_constructs() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let root = std::env::temp_dir().join(format!("anole-ir-cache-suite-{unique}"));
    fs::create_dir(&root).unwrap();

    for (name, source) in [
        ("basic", include_str!("fixtures/ir/basic.anole")),
        ("expressions", include_str!("fixtures/ir/expressions.anole")),
        ("control", include_str!("fixtures/ir/control.anole")),
        ("types", include_str!("fixtures/ir/types.anole")),
    ] {
        let directory = root.join(name);
        fs::create_dir(&directory).unwrap();
        let path = directory.join(format!("{name}.anole"));
        fs::write(&path, source).unwrap();
        let first = Command::new(env!("CARGO_BIN_EXE_anole"))
            .arg(&path)
            .output()
            .unwrap();
        assert!(first.stderr.is_empty(), "{name} source execution");

        fs::write(&path, "this source must not be parsed;").unwrap();
        File::open(&path)
            .unwrap()
            .set_times(
                FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)),
            )
            .unwrap();
        let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
            .arg(&path)
            .output()
            .unwrap();
        assert_eq!(cached.stdout, first.stdout, "{name} cached stdout");
        assert!(
            cached.stderr.is_empty(),
            "{name} cached stderr: {}",
            String::from_utf8_lossy(&cached.stderr)
        );
    }
    fs::remove_dir_all(root).unwrap();
}

#[test]
fn executes_newer_ir_caches_for_imported_modules() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-import-cache-{unique}"));
    fs::create_dir(&directory).unwrap();
    let main = directory.join("imports.anole");
    let dependency = directory.join("dep.anole");
    let operators = directory.join("operators.anole");
    fs::write(&main, include_str!("fixtures/ir/imports.anole")).unwrap();
    fs::write(&dependency, include_str!("fixtures/ir/dep.anole")).unwrap();
    fs::write(&operators, include_str!("fixtures/ir/operators.anole")).unwrap();

    let first = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(&directory)
        .arg(&main)
        .output()
        .unwrap();
    assert!(first.stderr.is_empty());

    for path in [&main, &dependency, &operators] {
        fs::write(path, "this source must not be parsed;").unwrap();
        File::open(path)
            .unwrap()
            .set_times(
                FileTimes::new().set_modified(UNIX_EPOCH + std::time::Duration::from_secs(1)),
            )
            .unwrap();
    }
    let cached = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(&directory)
        .arg(&main)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert_eq!(cached.stdout, first.stdout);
    assert!(
        cached.stderr.is_empty(),
        "{}",
        String::from_utf8_lossy(&cached.stderr)
    );
}

#[test]
fn writes_expected_ir_across_language_constructs() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-suite-{unique}"));
    fs::create_dir(&directory).unwrap();

    for name in ["expressions", "control", "types"] {
        assert_ir_fixture(&directory, name);
    }

    fs::write(
        directory.join("dep.anole"),
        include_str!("fixtures/ir/dep.anole"),
    )
    .unwrap();
    fs::write(
        directory.join("operators.anole"),
        include_str!("fixtures/ir/operators.anole"),
    )
    .unwrap();
    assert_ir_fixture(&directory, "imports");
    assert_ir_bytes(&directory.join("dep.anole"), "dep");
    assert_ir_bytes(&directory.join("operators.anole"), "operators");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn does_not_write_ir_when_execution_fails() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-error-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("error.anole");
    fs::write(&path, "@before: 1; missing();").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(!output.stderr.is_empty());
    assert!(!path.with_extension("anole.ir").exists());
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn reports_sidecar_filesystem_status_errors_before_execution() {
    use std::os::unix::fs::symlink;

    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-status-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("main.anole");
    fs::write(&path, "println(42);").unwrap();
    let ir_path = path.with_extension("anole.ir");
    symlink("main.anole.ir", &ir_path).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.stdout.is_empty());
    assert_eq!(
        output.stderr,
        format!(
            "filesystem error: status: Too many levels of symbolic links [{}]\n",
            ir_path.display()
        )
        .as_bytes()
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn ignores_sidecar_write_failures_and_prints_debug_ir_from_memory() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-sidecar-failure-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("main.anole");
    fs::write(&path, "println(42);").unwrap();
    let ir_path = path.with_extension("anole.ir");
    fs::create_dir(&ir_path).unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("-r")
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"42\n");
    assert!(output.stderr.is_empty());
    assert!(ir_path.is_dir());
    assert!(path.with_extension("anole.rd").is_file());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn exit_stops_before_the_ir_cache_is_written() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-ir-exit-{unique}"));
    fs::create_dir(&directory).unwrap();
    let path = directory.join("exit.anole");
    fs::write(&path, "print(\"before\"); exit(); print(\"after\");").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&path)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"before");
    assert!(output.stderr.is_empty());
    assert!(!path.with_extension("anole.ir").exists());
    fs::remove_dir_all(directory).unwrap();
}

fn assert_ir_fixture(directory: &std::path::Path, name: &str) {
    let source = match name {
        "expressions" => include_str!("fixtures/ir/expressions.anole"),
        "control" => include_str!("fixtures/ir/control.anole"),
        "types" => include_str!("fixtures/ir/types.anole"),
        "imports" => include_str!("fixtures/ir/imports.anole"),
        _ => unreachable!(),
    };
    let path = directory.join(format!("{name}.anole"));
    fs::write(&path, source).unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(directory)
        .arg(&path)
        .output()
        .unwrap();
    assert!(
        output.status.success(),
        "{name} failed: {}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert!(
        output.stderr.is_empty(),
        "{name}: {}",
        String::from_utf8_lossy(&output.stderr)
    );
    assert_ir_bytes(&path, name);
}

fn assert_ir_bytes(path: &std::path::Path, name: &str) {
    let expected = match name {
        "expressions" => include_str!("fixtures/ir/expressions.ir.hex"),
        "control" => include_str!("fixtures/ir/control.ir.hex"),
        "types" => include_str!("fixtures/ir/types.ir.hex"),
        "imports" => include_str!("fixtures/ir/imports.ir.hex"),
        "dep" => include_str!("fixtures/ir/dep.ir.hex"),
        "operators" => include_str!("fixtures/ir/operators.ir.hex"),
        _ => unreachable!(),
    };
    let actual = fs::read(path.with_extension("anole.ir")).unwrap();
    assert_eq!(actual, decode_hex(&expected.replace('\n', "")), "{name}");
}

fn decode_hex(hex: &str) -> Vec<u8> {
    hex.as_bytes()
        .chunks_exact(2)
        .map(|pair| {
            let pair = std::str::from_utf8(pair).unwrap();
            u8::from_str_radix(pair, 16).unwrap()
        })
        .collect()
}

#[test]
fn executes_piped_standard_input() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_anole"))
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .unwrap();
    use std::io::Write;
    child
        .stdin
        .take()
        .unwrap()
        .write_all(b"println(21 * 2);")
        .unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(output.status.success());
    assert_eq!(
        String::from_utf8(output.stdout).unwrap(),
        concat!(
            "    _                _\n",
            "   / \\   _ __   ___ | | ___\n",
            "  / _ \\ | '_ \\ / _ \\| |/ _ \\\n",
            " / ___ \\| | | | (_) | |  __/   0.0.24 2021/12/12\n",
            "/_/   \\_\\_| |_|\\___/|_|\\___|\n\n",
            ">> println(21 * 2);\n",
            "42\n",
            ">> ",
        )
    );
}

#[test]
fn input_preserves_the_carriage_return_from_crlf() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-input-crlf-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("input.anole");
    fs::write(&script, "println(input());").unwrap();
    let mut child = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&script)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .unwrap();
    use std::io::Write as _;
    child.stdin.take().unwrap().write_all(b"value\r\n").unwrap();
    let output = child.wait_with_output().unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"value\r\n");
    assert!(output.stderr.is_empty());
}

#[test]
fn repl_executes_only_the_first_statement_from_each_prompt() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_anole"))
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .unwrap();
    use std::io::Write;
    child
        .stdin
        .take()
        .unwrap()
        .write_all(b"println(1); println(2);\n")
        .unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(output.status.success());
    let stdout = String::from_utf8(output.stdout).unwrap();
    assert!(stdout.contains(">> println(1); println(2);\n1\n>> "));
    assert!(!stdout.contains("\n2\n"));
}

#[test]
fn repl_keeps_appended_code_live_for_captured_continuations() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_anole"))
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .unwrap();
    use std::io::Write;
    child
        .stdin
        .take()
        .unwrap()
        .write_all(
            concat!(
                "@saved: none;\n",
                "@round: 0;\n",
                "@value: call_with_current_continuation(@(continuation) { saved: continuation; return 10; });\n",
                "println(value);\n",
                "if round = 0 { round: 1; saved(20); }\n",
                // The parser consumes one lookahead line while checking for
                // `elif`/`else`; that line is not compiled separately.
                "println(999);\n",
            )
            .as_bytes(),
        )
        .unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(output.status.success());
    let stdout = String::from_utf8(output.stdout).unwrap();
    let first = stdout.find("\n10\n").expect("initial continuation value");
    let resumed = stdout.find("\n20\n").expect("resumed continuation value");
    assert!(first < resumed);
}

#[test]
fn streams_output_before_runtime_errors_and_before_input() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-streaming-{unique}"));
    fs::create_dir(&directory).unwrap();

    let failing = directory.join("failing.anole");
    fs::write(&failing, "print(\"before\"); missing + 1;").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&failing)
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"before");
    assert!(!output.stderr.is_empty());

    let prompting = directory.join("prompting.anole");
    fs::write(&prompting, "print(\"name? \"); println(input());").unwrap();
    let mut child = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&prompting)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .unwrap();
    use std::io::Write;
    child.stdin.take().unwrap().write_all(b"Ada\n").unwrap();
    let output = child.wait_with_output().unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert!(output.status.success());
    assert_eq!(output.stdout, b"name? Ada\n");
}

#[test]
fn tracebacks_include_the_full_call_chain_and_legacy_styling() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-traceback-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("traceback.anole");
    fs::write(
        &script,
        concat!(
            "@inner(value) {\n",
            "    return value.missing;\n",
            "}\n",
            "@middle(value): inner(value);\n",
            "@outer(): middle(1);\n",
            "outer();\n",
        ),
    )
    .unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&script)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert_eq!(
        String::from_utf8(output.stderr).unwrap(),
        concat!(
            "\u{1b}[1mTraceback:\n",
            "\u{1b}[0m  running at traceback.anole:6:6\n",
            "  running at traceback.anole:5:17\n",
            "  running at traceback.anole:4:22\n",
            "\u{1b}[1m  running at traceback.anole:2:17: \u{1b}[0m",
            "\u{1b}[31merror: \u{1b}[0mno member named missing\n",
        )
    );
}

#[test]
fn parser_errors_keep_the_source_line_and_legacy_styling() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-parser-error-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("invalid.anole");
    fs::write(&script, "@function() { return }").unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&script)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(output.status.success());
    assert!(output.stdout.is_empty());
    assert_eq!(
        String::from_utf8(output.stderr).unwrap(),
        format!(
            "\u{1b}[1m{}:1:22: \u{1b}[0m\u{1b}[31merror: \u{1b}[0mexpected an expr here\n\
             @function() {{ return }}\n                     \u{1b}[31m^\u{1b}[0m\n",
            script.display(),
        )
    );
}

#[test]
fn executes_prior_statements_before_a_later_lex_error() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-late-lex-error-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("invalid.anole");
    fs::write(&script, "print(\"before\"); println(..);").unwrap();

    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&script)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();

    assert!(output.status.success());
    assert_eq!(output.stdout, b"before");
    assert_eq!(output.stderr, b"unexpected \"..\"\n");
}

#[test]
fn installed_binary_uses_embedded_standard_modules() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-embedded-{unique}"));
    fs::create_dir(&directory).unwrap();
    let script = directory.join("embedded.anole");
    fs::write(
        &script,
        r#"
use env;
use coroutine;
println(env.args().size());
@once() { println(42); coroutine.co_yield(); }
@id: coroutine.co_create(once);
coroutine.co_resume(id);
coroutine.co_destroy(id);
"#,
    )
    .unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .current_dir(&directory)
        .arg(&script)
        .output()
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert!(output.status.success());
    assert_eq!(String::from_utf8(output.stdout).unwrap(), "1\n42\n");
    assert!(output.stderr.is_empty());
}

#[test]
fn repl_mode_prints_expression_results_and_keeps_state() {
    let mut interpreter = anole::Interpreter::new();
    assert_eq!(interpreter.run_repl("a: 40").unwrap(), "40\n");
    assert_eq!(interpreter.run_repl("a + 2").unwrap(), "42\n");
    assert_eq!(interpreter.run_repl("@b: 1").unwrap(), "");
}

#[test]
fn repl_reports_a_syntax_error_before_balancing_delimiters() {
    let interpreter = anole::Interpreter::new();
    assert!(interpreter.repl_input_complete("println((1);\n"));
    assert!(interpreter.repl_input_complete("println(..);\n"));
    assert!(!interpreter.repl_input_complete("println((1\n"));
    assert!(!interpreter.repl_input_complete("@function() {\n"));
}

#[test]
fn executes_directory_modules_and_writes_requested_debug_output() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-directory-{unique}"));
    fs::create_dir(&directory).unwrap();
    let source = directory.join("__init__.anole");
    fs::write(&source, "println(42);").unwrap();
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("-r")
        .arg(&directory)
        .output()
        .unwrap();
    let debug_output = source.with_extension("anole.rd");
    assert!(debug_output.is_file());
    let listing = fs::read_to_string(&debug_output).unwrap();
    assert!(listing.starts_with("Constants:\nCI"));
    assert!(listing.contains("\nInstructions:\n"));
    assert!(listing.contains("FastCall"));
    assert!(!listing.contains("Declaration("));
    fs::remove_dir_all(directory).unwrap();
    assert!(output.status.success());
    assert_eq!(String::from_utf8(output.stdout).unwrap(), "42\n");
}

#[test]
fn rejects_options_without_a_script() {
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("--unknown")
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(
        String::from_utf8(output.stdout).unwrap(),
        "invalid command-line argument(s)\n"
    );
}
