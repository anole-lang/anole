use anole::Parser;
use anole::ast::{Binding, Expr, Stmt};

#[test]
fn parses_function_declarations_and_control_flow() {
    let source = r#"
@sum(a, b: 2) { return a + b; }
@result: 0;
foreach [1, 2] as item {
    if item > 1 { result: sum(item); }
}
"#;
    let program = Parser::new(source, "<test>").unwrap().parse().unwrap();
    assert_eq!(program.len(), 3);
    let Stmt::Declaration(function) = &program[0] else {
        panic!("expected function declaration");
    };
    assert_eq!(
        function.bindings[0],
        Binding::Name {
            name: "sum".to_owned(),
            by_reference: true
        }
    );
    let Expr::Lambda { parameters, .. } = &function.values[0] else {
        panic!("expected lambda");
    };
    assert_eq!(parameters.len(), 2);
    assert!(parameters[1].default.is_some());
    assert!(matches!(program[2], Stmt::Foreach { .. }));
}

#[test]
fn dynamic_operators_affect_following_expressions() {
    let source = r#"
@*=*(left, right): left + right;
infixop 50 *=*;
@refof(&value): delay value;
prefixop refof;
result: refof 2 * 3 *=* 4 * 5;
"#;
    let program = Parser::new(source, "<test>").unwrap().parse().unwrap();
    assert_eq!(program.len(), 5);
    let Stmt::Expression(Expr::Binary { operator, .. }) = &program[4] else {
        panic!("expected final assignment");
    };
    assert_eq!(operator, ":");
}

#[test]
fn parses_classes_enums_match_and_destructuring() {
    let source = r#"
State: enum { Start, Running: 4, End };
class Base { value: 1; __init__(self, x) { self.value: x; } }
@[a, b]: [1, 2];
result: match State.Start { State.Start => { return a; }, => b };
"#;
    let program = Parser::new(source, "<test>").unwrap().parse().unwrap();
    assert_eq!(program.len(), 4);
    assert!(matches!(program[1], Stmt::Declaration(_)));
    let Stmt::Declaration(declaration) = &program[2] else {
        panic!("expected destructuring declaration");
    };
    assert!(matches!(declaration.bindings[0], Binding::Destructure(_)));
}

#[test]
fn enum_value_overflow_is_an_explicit_parse_error() {
    let error = Parser::new("@State: enum { Maximum: 9223372036854775807 };", "<test>")
        .unwrap()
        .parse()
        .unwrap_err();
    assert_eq!(error.message, "enum value overflow");
}

#[test]
fn float_literal_overflow_matches_stod_failure() {
    let source = format!("{}.0;", "9".repeat(400));
    let error = Parser::new(&source, "<test>").unwrap().parse().unwrap_err();
    assert_eq!(error.message, "stod");
}

#[test]
fn anonymous_lambda_shorthand_accepts_only_one_return_expression() {
    let error = anole::Parser::new("@f: @(): 1, 2;", "lambda.anole")
        .unwrap()
        .parse()
        .unwrap_err();
    assert_eq!(error.message, "wrong token here");
    assert_eq!(
        error.location,
        anole::Location {
            line: 1,
            column: 10
        }
    );
}
