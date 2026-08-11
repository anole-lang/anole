use std::fs;
use std::time::{SystemTime, UNIX_EPOCH};

use anole::Interpreter;

#[test]
fn imports_named_modules_aliases_and_all_exports() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-module-{unique}"));
    fs::create_dir(&directory).unwrap();
    fs::write(
        directory.join("numbers.anole"),
        "@one: 1; @two: 2; @sum(a, b): a + b;",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let source = r#"
use numbers as nums;
use one, sum from numbers;
println(nums.two);
println(sum(one, nums.two));
"#;
    let output = Interpreter::new()
        .run(source, &main.display().to_string())
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "2\n3\n");
}

#[test]
fn env_module_exposes_script_arguments_without_cpp_plugins() {
    let output = Interpreter::with_arguments(vec!["alpha".to_owned(), "beta".to_owned()])
        .run("use env; println(env.args());", "example/test.anole")
        .unwrap();
    assert_eq!(output, "[alpha, beta]\n");
}

#[test]
fn file_module_uses_the_rust_standard_library_backend() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let path = std::env::temp_dir().join(format!("anole-file-{unique}.txt"));
    let source = format!(
        r#"
use file;
f: file.open("{}", file.mode.out | file.mode.trunc);
f.write("hello");
f.close();
f: file.open("{}", file.mode.in);
println(f.readline());
println(f.eof());
f.read();
println(f.eof());
"#,
        path.display(),
        path.display()
    );
    let output = Interpreter::new()
        .run(&source, "example/file-test.anole")
        .unwrap();
    fs::remove_file(path).unwrap();
    assert_eq!(output, "hello\nfalse\ntrue\n");
}

#[test]
fn os_path_and_read_dir_modules_return_path_values() {
    let output = Interpreter::new()
        .run(
            "use os; println(os.path.is_directory(os.path.current_path()));",
            "example/os-test.anole",
        )
        .unwrap();
    assert_eq!(output, "true\n");
}
