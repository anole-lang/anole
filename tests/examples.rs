use std::fs;

use anole::Interpreter;

fn run_example(name: &str) -> String {
    let path = format!("example/{name}.anole");
    let source = fs::read_to_string(&path).unwrap();
    Interpreter::new().run(&source, &path).unwrap()
}

#[test]
fn repository_examples_match_the_legacy_outputs() {
    let cases = [
        ("class", "42\n"),
        ("codemo", "1\n1\n2\n2\n3\n3\n4\n4\n5\n5\n"),
        ("eval", "1\n2\n"),
        ("foreach", "01234567891\n2\n3\n[10, 10, 10]\n"),
        ("multiretval", "1\n2\n2\n1\n1\n2\n"),
        ("reference", "2 2 2 2 "),
        ("stream", "[10]\n"),
    ];
    for (name, expected) in cases {
        assert_eq!(run_example(name), expected, "example/{name}.anole");
    }
}

#[test]
fn parsec_example_preserves_the_legacy_runtime_error_after_imported_operators_parse() {
    let path = "example/parsec/demo.anole";
    let source = fs::read_to_string(path).unwrap();
    let error = Interpreter::new().run(&source, path).unwrap_err();
    assert_eq!(error.message, "no member named index");
}
