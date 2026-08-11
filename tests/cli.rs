use std::fs;
use std::process::{Command, Stdio};
use std::time::{SystemTime, UNIX_EPOCH};

#[test]
fn reports_the_legacy_version_literal() {
    let output = Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg("--version")
        .output()
        .unwrap();
    assert!(output.status.success());
    assert_eq!(
        String::from_utf8(output.stdout).unwrap(),
        "Anole 0.0.24 2021/12/12\n"
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
    fs::remove_file(path).unwrap();
    assert!(output.status.success());
    assert_eq!(String::from_utf8(output.stdout).unwrap(), "42\n");
    assert!(output.stderr.is_empty());
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
    assert_eq!(String::from_utf8(output.stdout).unwrap(), "42\n");
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
id: coroutine.co_create(once);
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
