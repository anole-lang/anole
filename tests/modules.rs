use std::fs::{self, File, FileTimes};
use std::time::{SystemTime, UNIX_EPOCH};

#[cfg(unix)]
use std::ffi::OsString;
#[cfg(unix)]
use std::os::unix::ffi::OsStringExt;

use anole::Interpreter;

fn temporary_module_directory(label: &str) -> std::path::PathBuf {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let directory = std::env::temp_dir().join(format!("anole-{label}-{unique}"));
    fs::create_dir(&directory).unwrap();
    directory
}

#[test]
fn imports_named_modules_aliases_and_all_exports() {
    let directory = temporary_module_directory("module");
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

#[cfg(unix)]
#[test]
fn direct_module_paths_preserve_non_utf8_source_bytes() {
    let directory = temporary_module_directory("raw-module-path");
    let module_name = OsString::from_vec(b"raw-\x80.anole".to_vec());
    fs::write(directory.join(&module_name), b"@answer: 42;").unwrap();
    let main = directory.join("main.anole");
    let source = b"use \"raw-\x80.anole\" as raw; println(raw.answer);";
    fs::write(&main, source).unwrap();

    let output = Interpreter::new().run_file_bytes(source, &main).unwrap();

    assert_eq!(output, "42\n");
    let mut module_ir = module_name;
    module_ir.push(".ir");
    assert!(directory.join(module_ir).is_file());
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn named_modules_preserve_non_utf8_identifier_bytes() {
    let directory = temporary_module_directory("raw-named-module");
    let module_name = OsString::from_vec(b"\x80.anole".to_vec());
    fs::write(directory.join(&module_name), b"@answer: 42;").unwrap();
    let main = directory.join("main.anole");
    let source = b"use \x80; println(\x80.answer);";

    let output = Interpreter::new().run_file_bytes(source, &main).unwrap();

    assert_eq!(output, "42\n");
    let mut module_ir = module_name;
    module_ir.push(".ir");
    assert!(directory.join(module_ir).is_file());
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn named_and_direct_directory_imports_take_different_paths() {
    let directory = temporary_module_directory("directory-import-paths");
    let named = directory.join("named_empty");
    fs::create_dir(&named).unwrap();
    let main = directory.join("main.anole");

    let error = Interpreter::new()
        .run("use named_empty;", &main.display().to_string())
        .unwrap_err();
    assert_eq!(
        error.message,
        format!(
            "cannot open file {}",
            named.join("__init__.anole").display()
        )
    );

    let direct = directory.join("direct_package");
    fs::create_dir(&direct).unwrap();
    fs::write(
        direct.join("__init__.anole"),
        "print(\"init-ran\"); @answer: 42;",
    )
    .unwrap();
    let output = Interpreter::new()
        .run(
            "use \"direct_package\" as direct; println(\"after\");",
            &main.display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "after\n");
    assert!(directory.join("direct_package.ir").is_file());

    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn unreadable_direct_directory_imports_fail_to_open() {
    use std::os::unix::fs::PermissionsExt;

    let directory = temporary_module_directory("unreadable-direct-directory");
    let blocked = directory.join("blocked");
    fs::create_dir(&blocked).unwrap();
    fs::set_permissions(&blocked, fs::Permissions::from_mode(0o0)).unwrap();
    let main = directory.join("main.anole");

    let error = Interpreter::new()
        .run("use \"blocked\" as blocked;", &main.display().to_string())
        .unwrap_err();

    fs::set_permissions(&blocked, fs::Permissions::from_mode(0o700)).unwrap();
    assert_eq!(
        error.message,
        format!("cannot open file {}", blocked.display())
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn nested_imports_name_the_non_module_part() {
    let directory = temporary_module_directory("non-module-import-part");
    fs::write(directory.join("plain.anole"), "@answer: 42;").unwrap();
    let main = directory.join("main.anole");
    for source in ["use plain.answer.missing;", "use * from plain.answer;"] {
        let error = Interpreter::new()
            .run(source, &main.display().to_string())
            .unwrap_err();
        assert_eq!(error.message, "answer is not a module");
    }
    fs::remove_dir_all(directory).unwrap();
}

// Module source is parsed one statement at a time so imports can add operators
// before the parser reaches their uses.
#[test]
fn imported_operators_affect_later_module_statements() {
    let directory = temporary_module_directory("module-operator");
    fs::write(
        directory.join("operators.anole"),
        "@*~*(left, right): left + right; infixop 180 *~*;",
    )
    .unwrap();
    fs::write(
        directory.join("consumer.anole"),
        "use * from operators; @answer: 20 *~* 22;",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            "use consumer; println(consumer.answer);",
            &main.display().to_string(),
        )
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "42\n");
}

#[test]
fn imported_equal_precedence_operators_keep_declaration_order() {
    let directory = temporary_module_directory("module-equal-operator-order");
    fs::write(
        directory.join("operators.anole"),
        concat!(
            "@first(left, right): left * 10 + right; infixop 50 first;",
            "@second(left, right): left * 100 + right; infixop 50 second;",
        ),
    )
    .unwrap();
    fs::write(
        directory.join("consumer.anole"),
        "use * from operators; @answer: 1 second 2 first 3;",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            "use consumer; println(consumer.answer);",
            &main.display().to_string(),
        )
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "123\n");
}

#[test]
fn incremental_code_remains_live_for_captured_continuations() {
    let directory = temporary_module_directory("module-live-code-continuation");
    fs::write(
        directory.join("operators.anole"),
        "@*~*(left, right): left + right; infixop 180 *~*;",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let source = concat!(
        "use * from operators;",
        "@saved: none; @round: 0;",
        "@value: call_with_current_continuation(@(continuation) {",
        "saved: continuation; return 10; });",
        "println(value);",
        "if round = 0 { round: 1; saved(20); }",
    );
    let output = Interpreter::new()
        .run(source, &main.display().to_string())
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "10\n20\n");
}

#[test]
fn continuations_work_at_module_top_level() {
    let directory = temporary_module_directory("module-continuation");
    fs::write(
        directory.join("answer.anole"),
        r#"
@mapping: dict {
    "value" => call_with_current_continuation(@(continuation): continuation(42))
};
@value: mapping.value;
"#,
    )
    .unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            "use answer; println(answer.value);",
            &main.display().to_string(),
        )
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "42\n");
}

#[test]
fn top_level_module_return_resumes_and_advances_the_importer_context() {
    let directory = temporary_module_directory("module-top-level-return");
    fs::write(
        directory.join("dep.anole"),
        "@before: 1; return 42; @after: 2;",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let source = concat!(
        "use dep;",
        "@observed: 99;",
        "println(type(observed));",
        "println(type(dep));",
        "use dep;",
        "println(type(dep));",
    );

    let output = Interpreter::new()
        .run(source, &main.display().to_string())
        .unwrap();
    assert_eq!(output, "anolemodule\ninteger\nanolemodule\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn modules_do_not_inherit_callers_user_variables() {
    let directory = temporary_module_directory("module-scope");
    fs::write(directory.join("isolated.anole"), "@revealed: secret;").unwrap();
    let main = directory.join("main.anole");
    let error = Interpreter::new()
        .run("@secret: 42; use isolated;", &main.display().to_string())
        .unwrap_err();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(
        error.message,
        "var named secret doesn't reference to any object"
    );
}

#[test]
fn modules_export_only_symbols_in_their_final_local_scope() {
    let directory = temporary_module_directory("module-local-exports");
    fs::write(directory.join("plain.anole"), "@answer: 42;").unwrap();
    fs::write(
        directory.join("continued.anole"),
        concat!(
            "@saved: none; @round: 0; @before: 1;",
            "@value: call_with_current_continuation(@(continuation) {",
            "saved: continuation; return 10; });",
            "if round = 0 { round: 1; saved(20); }",
            "@after: 2;",
        ),
    )
    .unwrap();
    let main = directory.join("main.anole");

    for source in [
        "use plain; plain.println(1);",
        "use continued; println(continued.before);",
    ] {
        let error = Interpreter::new()
            .run(source, &main.display().to_string())
            .unwrap_err();
        assert!(
            matches!(
                error.message.as_str(),
                "no member named println" | "no member named before"
            ),
            "{}",
            error.message
        );
    }

    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn failed_module_initialization_is_not_cached() {
    let directory = temporary_module_directory("module-retry");
    let module = directory.join("broken.anole");
    fs::write(&module, "@partial: 1; missing + 1;").unwrap();
    let main = directory.join("main.anole");
    let mut interpreter = Interpreter::new();
    let first = interpreter
        .run("use broken;", &main.display().to_string())
        .unwrap_err();
    assert_eq!(
        first.message,
        "var named missing doesn't reference to any object"
    );

    fs::write(&module, "@answer: 42;").unwrap();
    let output = interpreter
        .run(
            "use broken; println(broken.answer);",
            &main.display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "42\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn lexically_normalizes_module_paths_before_reporting_errors() {
    let directory = temporary_module_directory("normalized-module-error");
    fs::create_dir(directory.join("nested")).unwrap();
    let main = directory.join("main.anole");
    let error = Interpreter::new()
        .run(
            "use \"nested/../missing.anole\" as missing;",
            &main.display().to_string(),
        )
        .unwrap_err();
    assert_eq!(
        error.message,
        format!(
            "cannot open file {}",
            directory.join("missing.anole").display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn lexically_normalized_module_paths_preserve_a_trailing_separator() {
    let directory = temporary_module_directory("normalized-module-trailing-separator");
    let main = directory.join("main.anole");

    for (source, missing) in [
        ("use \"missing/.\" as missing;", "missing"),
        ("use \"missing/child/..\" as missing;", "missing"),
        ("use \"missing.ext/\" as missing;", "missing.ext"),
    ] {
        let error = Interpreter::new()
            .run(source, &main.display().to_string())
            .unwrap_err();
        assert_eq!(
            error.message,
            format!("cannot open file {}/", directory.join(missing).display())
        );
    }

    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn trailing_directory_module_paths_keep_the_directory_as_the_code_root() {
    let directory = temporary_module_directory("trailing-directory-code-root");
    let template = directory.join("template.anole");
    fs::write(&template, "use dep; println(dep.answer);").unwrap();
    fs::write(directory.join("dep.anole"), "@answer: 42;").unwrap();
    let compiled = std::process::Command::new(env!("CARGO_BIN_EXE_anole"))
        .arg(&template)
        .output()
        .unwrap();
    assert_eq!(compiled.stdout, b"42\n");
    assert!(compiled.stderr.is_empty());

    let package = directory.join("package");
    fs::create_dir(&package).unwrap();
    fs::write(package.join("dep.anole"), "@answer: 99;").unwrap();
    let cache = package.join(".ir");
    fs::copy(template.with_extension("anole.ir"), &cache).unwrap();
    File::open(&cache)
        .unwrap()
        .set_times(
            FileTimes::new().set_modified(SystemTime::now() + std::time::Duration::from_secs(60)),
        )
        .unwrap();

    let output = Interpreter::new()
        .run(
            "use \"package/\" as package;",
            &directory.join("main.anole").display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "99\n");

    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn module_cache_keys_distinguish_a_trailing_directory_separator() {
    let directory = temporary_module_directory("trailing-directory-cache-key");
    fs::create_dir(directory.join("package")).unwrap();
    let output = Interpreter::new()
        .run(
            concat!(
                "use \"package/\" as with_separator;",
                "use \"package\" as without_separator;",
                "println(with_separator is without_separator);",
            ),
            &directory.join("main.anole").display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "false\n");
    assert!(directory.join("package/.ir").is_file());
    assert!(directory.join("package.ir").is_file());

    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn named_module_lookup_preserves_filesystem_status_errors() {
    use std::os::unix::fs::symlink;

    let directory = temporary_module_directory("named-module-status-error");
    let loop_path = directory.join("loop.anole");
    symlink("loop.anole", &loop_path).unwrap();
    let main = directory.join("main.anole");
    let error = Interpreter::new()
        .run("use loop;", &main.display().to_string())
        .unwrap_err();
    assert_eq!(
        error.to_string(),
        format!(
            "filesystem error: status: Too many levels of symbolic links [{}]",
            loop_path.display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn imported_module_sidecars_preserve_filesystem_status_errors() {
    use std::os::unix::fs::symlink;

    let directory = temporary_module_directory("module-sidecar-status-error");
    let module = directory.join("dependency.anole");
    fs::write(&module, "println(42);").unwrap();
    let sidecar = module.with_extension("anole.ir");
    symlink("dependency.anole.ir", &sidecar).unwrap();
    let main = directory.join("main.anole");
    let error = Interpreter::new()
        .run("use dependency;", &main.display().to_string())
        .unwrap_err();
    assert_eq!(
        error.to_string(),
        format!(
            "filesystem error: status: Too many levels of symbolic links [{}]",
            sidecar.display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn module_cache_keys_are_lexical_and_do_not_resolve_symlinks() {
    use std::os::unix::fs::symlink;

    let directory = temporary_module_directory("module-symlink");
    fs::write(directory.join("real.anole"), "print(\"loaded\");").unwrap();
    symlink("real.anole", directory.join("alias.anole")).unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            concat!(
                "use \"real.anole\" as real;",
                "use \"alias.anole\" as alias;",
            ),
            &main.display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "loadedloaded");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn modules_have_independent_constant_pools() {
    let directory = temporary_module_directory("module-constant-pools");
    fs::write(directory.join("left.anole"), "@value: 7;").unwrap();
    fs::write(directory.join("right.anole"), "@value: 7;").unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            "use left, right; println(left.value is right.value);",
            &main.display().to_string(),
        )
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(output, "false\n");
}

#[test]
fn repeated_imports_share_module_identity_but_not_variable_slots() {
    let directory = temporary_module_directory("module-import-slots");
    fs::write(directory.join("plain.anole"), "@answer: 42;").unwrap();
    let main = directory.join("main.anole");
    let output = Interpreter::new()
        .run(
            concat!(
                "use plain; @&first: plain;",
                "use plain; @&second: plain;",
                "println(first is second);",
                "first: 1;",
                "println(type(second)); println(second.answer);",
            ),
            &main.display().to_string(),
        )
        .unwrap();
    assert_eq!(output, "true\nanolemodule\n42\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn tracebacks_use_the_imported_module_source_locations() {
    let directory = temporary_module_directory("module-traceback");
    fs::write(
        directory.join("failing.anole"),
        "@inner(value): value.missing;\ninner(1);\n",
    )
    .unwrap();
    let main = directory.join("main.anole");
    let error = Interpreter::new()
        .run("use failing;", &main.display().to_string())
        .unwrap_err();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(
        error.to_string(),
        concat!(
            "\u{1b}[1mTraceback:\n",
            "\u{1b}[0m  running at failing.anole:2:6\n",
            "\u{1b}[1m  running at failing.anole:1:21: \u{1b}[0m",
            "\u{1b}[31merror: \u{1b}[0mno member named missing",
        )
    );
}

#[test]
fn env_module_exposes_script_arguments() {
    let output = Interpreter::with_arguments(vec!["alpha".to_owned(), "beta".to_owned()])
        .run("use env; println(env.args());", "example/test.anole")
        .unwrap();
    assert_eq!(output, "[alpha, beta]\n");
}

#[test]
fn standard_modules_preserve_their_anole_wrapper_scopes() {
    let output = Interpreter::new()
        .run(
            concat!(
                "use env, file, os;",
                "println(type(env.__args)); println(type(env.args));",
                "println(type(file.__open)); println(type(file.open));",
                "println(type(file.mode)); println(file.mode.in | file.mode.out);",
                "println(type(os.path.__current_path));",
                "println(type(os.path.current_path));",
                "println(type(os.read_dir.__read_dir));",
                "println(type(os.read_dir.read_dir));",
            ),
            "example/standard-module-wrappers.anole",
        )
        .unwrap();
    assert_eq!(
        output,
        concat!(
            "builtinfunc\nfunc\n",
            "builtinfunc\nfunc\nfunc\n12\n",
            "builtinfunc\nfunc\n",
            "builtinfunc\nfunc\n",
        )
    );
}

#[test]
fn standard_module_wrappers_enforce_function_arity() {
    for (source, expected) in [
        (
            "use env; env.args(1);",
            "function takes 0 arguments but 1 were given",
        ),
        ("use env; env.__args(1);", "args need no arguments"),
        (
            "use file; file.open();",
            "missing the parameter named 'path'",
        ),
        (
            "use file; file.open(1, 2, 3);",
            "function takes 2 arguments but 3 were given",
        ),
        (
            "use file; file.__open(1);",
            "function open need 2 arguments",
        ),
        (
            "use os; os.path.current_path(1);",
            "function takes 0 arguments but 1 were given",
        ),
        (
            "use os; os.path.is_directory();",
            "missing the parameter named 'path'",
        ),
        (
            "use os; os.path.__current_path(1);",
            "function current_path need no arguments",
        ),
        (
            "use os; os.path.__is_directory();",
            "function current_path need 1 argument",
        ),
        (
            "use os; os.read_dir.read_dir();",
            "missing the parameter named 'path'",
        ),
        (
            "use os; os.read_dir.__read_dir();",
            "function read_dir need only one argument",
        ),
    ] {
        let error = Interpreter::new()
            .run(source, "example/standard-module-arity.anole")
            .unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
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
    assert_eq!(output, "hello\ntrue\ntrue\n");
}

#[test]
fn file_write_preserves_string_bytes() {
    let directory = temporary_module_directory("file-binary-write");
    let input = directory.join("input.bin");
    let output = directory.join("output.bin");
    fs::write(&input, [0x80, 0xff]).unwrap();
    let source = format!(
        concat!(
            "use file;",
            "@input: file.open(\"{}\", file.mode.in | file.mode.binary);",
            "@output: file.open(\"{}\", file.mode.out | file.mode.binary);",
            "output.write(input.read()); output.write(input.read()); output.close();",
        ),
        input.display(),
        output.display(),
    );
    Interpreter::new()
        .run(&source, "example/file-binary-write.anole")
        .unwrap();
    assert_eq!(fs::read(&output).unwrap(), [0x80, 0xff]);
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn file_writes_remain_buffered_until_flush() {
    let directory = temporary_module_directory("buffered-file-write");
    let output = directory.join("buffered.txt");
    let source = format!(
        concat!(
            "use file;",
            "@writer: file.open(\"{}\", file.mode.out);",
            "writer.write(\"x\", none);",
            "println(writer.tellp());",
            "@before: file.open(\"{}\", file.mode.in);",
            "before.read(); println(before.eof());",
            "writer.flush(none);",
            "@after: file.open(\"{}\", file.mode.in);",
            "println(after.read());",
        ),
        output.display(),
        output.display(),
        output.display(),
    );

    let rendered = Interpreter::new()
        .run(&source, "example/buffered-file-write.anole")
        .unwrap();

    assert_eq!(rendered, "1\ntrue\nx\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn file_reads_use_a_read_ahead_buffer_and_logical_position() {
    let directory = temporary_module_directory("file-read-ahead");
    let path = directory.join("read-ahead.txt");
    fs::write(&path, "ab").unwrap();
    let source = format!(
        concat!(
            "use file;",
            "@reader: file.open(\"{}\", file.mode.in);",
            "print(reader.read()); println(reader.tellg());",
            "@writer: file.open(\"{}\", file.mode.in | file.mode.out);",
            "writer.seekp(1, none); writer.write(\"x\", none); writer.flush(none);",
            "print(reader.read());",
        ),
        path.display(),
        path.display(),
    );

    let rendered = Interpreter::new()
        .run(&source, "example/file-read-ahead.anole")
        .unwrap();

    assert_eq!(rendered, "a1\nb");
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn posix_filesystem_calls_truncate_paths_at_embedded_nul_bytes() {
    let directory = temporary_module_directory("nul-path");
    let file_path = directory.join("truncated-file");
    let child_directory = directory.join("truncated-directory");
    fs::create_dir(&child_directory).unwrap();
    let source = format!(
        concat!(
            "use file; use os;",
            "@stream: file.open(\"{}\\0ignored\", file.mode.out);",
            "println(stream.good());",
            "println(os.path.is_directory(\"{}\\0ignored\"));",
        ),
        file_path.display(),
        child_directory.display(),
    );

    let output = Interpreter::new()
        .run(&source, "example/nul-path.anole")
        .unwrap();

    assert_eq!(output, "true\ntrue\n");
    assert!(file_path.is_file());
    fs::remove_dir_all(directory).unwrap();
}

#[cfg(unix)]
#[test]
fn direct_module_io_truncates_paths_at_embedded_nul_bytes() {
    let directory = temporary_module_directory("nul-module-path");
    let module = directory.join("nul-target");
    let main = directory.join("main.anole");
    let source = "use \"nul-target\\0ignored\" as target; println(target.answer);";
    fs::write(&module, "@answer: 42;").unwrap();

    let first = Interpreter::new()
        .run(source, &main.display().to_string())
        .unwrap();
    assert_eq!(first, "42\n");

    let cached = fs::read(&module).unwrap();
    assert_eq!(
        u64::from_ne_bytes(cached[..8].try_into().unwrap()),
        20_210_213
    );
    let second = Interpreter::new()
        .run(source, &main.display().to_string())
        .unwrap();
    assert_eq!(second, "42\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn file_streams_preserve_failed_open_and_eof_state() {
    let directory = temporary_module_directory("file-state");
    let missing = directory.join("missing.txt");
    let empty = directory.join("empty.txt");
    fs::write(&empty, "").unwrap();
    let source = format!(
        r#"
use file;
@missing: file.open("{}");
println(missing.good());
println(missing.eof());
println(missing.read().size());
println(missing.eof());
println(missing.good());

@input: file.open("{}", file.mode.in);
println(input.good());
println(input.readline());
println(input.eof());
println(input.read().size());
println(input.eof());
println(input.good());
"#,
        missing.display(),
        empty.display(),
    );
    let output = Interpreter::new()
        .run(&source, "example/file-state.anole")
        .unwrap();
    assert!(
        !missing.exists(),
        "default in|out mode must not create files"
    );
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(
        output,
        "false\nfalse\n1\nfalse\nfalse\ntrue\n\ntrue\n1\ntrue\nfalse\n"
    );
}

#[test]
fn file_readline_preserves_cr_and_sets_eof_on_an_unterminated_line() {
    let directory = temporary_module_directory("file-readline-state");
    let crlf = directory.join("crlf.txt");
    let unterminated = directory.join("unterminated.txt");
    fs::write(&crlf, b"first\r\nsecond\n").unwrap();
    fs::write(&unterminated, b"tail").unwrap();
    let source = format!(
        r#"
use file;
@crlf: file.open("{}", file.mode.in);
println(crlf.readline());
println(crlf.eof());
println(crlf.good());
println(crlf.readline());
println(crlf.eof());
println(crlf.good());
println(crlf.readline());
println(crlf.eof());
println(crlf.good());

@unterminated: file.open("{}", file.mode.in);
println(unterminated.readline());
println(unterminated.eof());
println(unterminated.good());
println(unterminated.tellg());
unterminated.seekg(0, none);
println(unterminated.eof());
println(unterminated.good());
println(unterminated.read().size());

@tellp_state: file.open("{}", file.mode.in | file.mode.out);
tellp_state.readline();
println(tellp_state.eof());
println(tellp_state.good());
println(tellp_state.tellp());
println(tellp_state.eof());
println(tellp_state.good());
tellp_state.seekg(0, none);
println(tellp_state.eof());
println(tellp_state.good());
println(tellp_state.read());

@seekp_state: file.open("{}", file.mode.in | file.mode.out);
seekp_state.readline();
seekp_state.seekp(0, none);
println(seekp_state.tellp());
println(seekp_state.eof());
println(seekp_state.good());
"#,
        crlf.display(),
        unterminated.display(),
        unterminated.display(),
        unterminated.display(),
    );
    let output = Interpreter::new()
        .run(&source, "example/file-readline-state.anole")
        .unwrap();
    fs::remove_dir_all(directory).unwrap();
    assert_eq!(
        output,
        concat!(
            "first\r\nfalse\ntrue\n",
            "second\nfalse\ntrue\n",
            "\ntrue\nfalse\n",
            "tail\ntrue\nfalse\n-1\n",
            "false\nfalse\n1\n",
            "true\nfalse\n4\ntrue\nfalse\n",
            "false\ntrue\nt\n",
            "0\ntrue\nfalse\n",
        )
    );
}

#[test]
fn file_void_methods_with_spare_stack_values_preserve_stream_state() {
    let directory = temporary_module_directory("file-void-method-state");
    let input = directory.join("input.txt");
    let written = directory.join("written.txt");
    fs::write(&input, b"ab").unwrap();
    let source = format!(
        r#"
use file;
@stateful: file.open("{}", file.mode.in | file.mode.out);
println(stateful.read());
println(stateful.read());
stateful.read();
println(stateful.eof());
println(stateful.good());
stateful.seekg(0, none);
println(stateful.eof());
println(stateful.good());
println(stateful.read().size());

@closed: file.open("{}", file.mode.in);
closed.close(none);
println(closed.good());
closed.read();
println(closed.eof());
println(closed.good());

@closed_tell: file.open("{}", file.mode.in | file.mode.out);
closed_tell.close(none);
println(closed_tell.tellg());
println(closed_tell.eof());
println(closed_tell.good());

@closed_tellp: file.open("{}", file.mode.in | file.mode.out);
closed_tellp.close(none);
println(closed_tellp.tellp());
println(closed_tellp.eof());
println(closed_tellp.good());

@negative_seek: file.open("{}", file.mode.in);
negative_seek.seekg(0 - 1, none);
println(negative_seek.eof());
println(negative_seek.good());
println(negative_seek.tellg());

@output: file.open("{}", file.mode.out);
output.write("written", none);
output.flush(none);
output.close(none);
"#,
        input.display(),
        input.display(),
        input.display(),
        input.display(),
        input.display(),
        written.display(),
    );
    let output = Interpreter::new()
        .run(&source, "example/file-void-method-state.anole")
        .unwrap();
    assert_eq!(
        output,
        concat!(
            "a\nb\ntrue\nfalse\n",
            "false\nfalse\n1\n",
            "true\ntrue\nfalse\n",
            "-1\nfalse\ntrue\n",
            "-1\nfalse\ntrue\n",
            "false\nfalse\n-1\n",
        )
    );
    assert_eq!(fs::read(&written).unwrap(), b"written");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn file_void_methods_leave_ignored_arguments_as_the_call_result() {
    let directory = temporary_module_directory("file-void-method-results");
    let path = directory.join("output.txt");
    let source = format!(
        r#"
use file;
@output: file.open("{}", file.mode.out);
println(output.write("x", 41));
println(output.flush(42));
println(output.seekp(0, 43));
println(output.close(44));

str(1, 77);
@first_residue: file.open("{}-residue-1", file.mode.out);
println(first_residue.close());
time(78);
@second_residue: file.open("{}-residue-2", file.mode.out);
println(second_residue.close());
[1].size(79);
@third_residue: file.open("{}-residue-3", file.mode.out);
println(third_residue.close());

@saved: none;
@round: 0;
@capture(continuation) {{ saved: continuation; return 1; }}
@answer: call_with_current_continuation(capture, 88);
if round = 0 {{ round: 1; saved(2); }}
println(answer);
@continuation_residue: file.open("{}-residue-cont", file.mode.out);
println(continuation_residue.close());
"#,
        path.display(),
        path.display(),
        path.display(),
        path.display(),
        path.display(),
    );

    let output = Interpreter::new()
        .run(&source, "example/file-void-method-results.anole")
        .unwrap();
    assert_eq!(output, "41\n42\n43\n44\n77\n78\n79\n2\n88\n");
    assert_eq!(fs::read(path).unwrap(), b"x");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn native_rust_values_have_stable_type_names() {
    let directory = temporary_module_directory("native-type-names");
    let path = directory.join("output.txt");
    let source = format!(
        concat!(
            "use file, os;",
            "@stream: file.open(\"{}\", file.mode.out);",
            "@path: os.path.current_path();",
            "class Typed {{ method(self): none; }}",
            "println(type(Typed));",
            "println(type(Typed.method));",
            "println(type(stream));",
            "println(type(path));",
        ),
        path.display(),
    );

    let output = Interpreter::new()
        .run(&source, "example/native-type-names.anole")
        .unwrap();
    assert_eq!(output, "class\nmethod\nfile\npath\n");
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn file_out_mode_truncates_an_existing_file_without_an_explicit_trunc_flag() {
    let directory = temporary_module_directory("file-out-truncates");
    let path = directory.join("content.txt");
    fs::write(&path, "existing content").unwrap();
    let source = format!(
        r#"use file; file.open("{}", file.mode.out);"#,
        path.display()
    );
    Interpreter::new()
        .run(&source, "example/file-out-truncates.anole")
        .unwrap();
    assert_eq!(fs::read(&path).unwrap(), b"");
    fs::remove_dir_all(directory).unwrap();
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

#[cfg(unix)]
#[test]
fn is_directory_preserves_filesystem_status_errors() {
    use std::os::unix::fs::PermissionsExt;

    let directory = temporary_module_directory("is-directory-permission");
    let blocked = directory.join("blocked");
    let child = blocked.join("child");
    fs::create_dir_all(&child).unwrap();
    fs::set_permissions(&blocked, fs::Permissions::from_mode(0o0)).unwrap();
    let source = format!(r#"use os; os.path.is_directory("{}");"#, child.display());
    let error = Interpreter::new()
        .run(&source, "example/is-directory-permission.anole")
        .unwrap_err();
    fs::set_permissions(&blocked, fs::Permissions::from_mode(0o700)).unwrap();
    assert_eq!(
        error.to_string(),
        format!(
            "filesystem error: status: Permission denied [{}]",
            child.display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn read_dir_lexically_normalizes_returned_paths() {
    let directory = temporary_module_directory("normalized-read-dir");
    let child = directory.join("only-child");
    fs::create_dir(&child).unwrap();
    let source = format!(
        r#"use os; println(os.read_dir.read_dir("{}/.")[0]);"#,
        directory.display()
    );
    let output = Interpreter::new()
        .run(&source, "example/normalized-read-dir.anole")
        .unwrap();
    assert_eq!(output, format!("{}\n", child.display()));
    fs::remove_dir_all(directory).unwrap();
}

#[test]
fn read_dir_preserves_the_filesystem_exception_diagnostic() {
    let directory = temporary_module_directory("missing-read-dir");
    let missing = directory.join("missing");
    let source = format!(r#"use os; os.read_dir.read_dir("{}");"#, missing.display());
    let error = Interpreter::new()
        .run(&source, "example/missing-read-dir.anole")
        .unwrap_err();
    assert_eq!(
        error.to_string(),
        format!(
            "filesystem error: directory iterator cannot open directory: No such file or directory [{}]",
            missing.display()
        )
    );
    fs::remove_dir_all(directory).unwrap();
}
