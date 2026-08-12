use anole::Interpreter;

fn execute(source: &str) -> String {
    Interpreter::new().run(source, "<test>").unwrap()
}

#[test]
fn runs_simple_assignments_and_arithmetic() {
    assert_eq!(
        execute(
            r#"
a: 1;
b: 2;
b: a : 3;
print(a + b);
"#,
        ),
        "6"
    );
}

#[test]
fn invalid_integer_arithmetic_reports_explicit_runtime_errors() {
    for (source, expected) in [
        ("@zero: 0; @one: 1; one / zero;", "integer division by zero"),
        ("@zero: 0; @one: 1; one % zero;", "integer division by zero"),
        (
            "@maximum: 9223372036854775807; @one: 1; maximum + one;",
            "integer overflow",
        ),
        ("9223372036854775807 + 1;", "integer overflow"),
        (
            "@distance: 64; @one: 1; one << distance;",
            "invalid shift count",
        ),
        (
            "@distance: 0 - 1; @one: 1; one >> distance;",
            "invalid shift count",
        ),
    ] {
        let error = Interpreter::new().run(source, "<test>").unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
}

#[test]
fn runs_curried_functions() {
    assert_eq!(
        execute(
            r#"
@adddd: @(a): @(b): @(c): @(d): a + b + c + d;
print(adddd(1)(2)(3)(4));
"#,
        ),
        "10"
    );
}

#[test]
fn supports_variadic_arguments_and_unpacking() {
    assert_eq!(
        execute(
            r#"
@foo(a, b, ...c) { return c; }
@l: [2, 3, 4, 5, 6];
println(foo(1, l..., 7));
"#,
        ),
        "[3, 4, 5, 6, 7]\n"
    );
}

#[test]
fn supports_default_arguments_at_call_time() {
    assert_eq!(
        execute(
            r#"
foo: @(m, n: n) { return m + n; };
n: 1;
println(foo(1));
n: 2;
println(foo(1));
"#,
        ),
        "2\n3\n"
    );
}

#[test]
fn foreach_variables_alias_list_elements() {
    assert_eq!(
        execute(
            r#"
l: [1, 2, 3];
foreach l as i { i: 10; }
println(l);
"#,
        ),
        "[10, 10, 10]\n"
    );
}

#[test]
fn custom_operators_follow_declared_precedence() {
    assert_eq!(
        execute(
            r#"
@*=*(lhs, rhs): lhs + rhs;
@*^*(lhs, rhs) { return lhs + rhs; }
infixop 50 *=*;
infixop 200 *^*;
println(2 * 3 *=* 4 * 5);
println(2 * 3 *^* 4 * 5);
"#,
        ),
        "26\n70\n"
    );
}

#[test]
fn keeps_equal_numeric_custom_precedences_in_distinct_layers() {
    assert_eq!(
        execute(
            r#"
@first(left, right): left * 10 + right;
@second(left, right): left * 100 + right;
infixop 50 first;
infixop 50 second;
println(1 second 2 first 3);
println(1 first 2 second 3);
"#,
        ),
        "123\n1203\n"
    );
}

#[test]
fn builtins_bind_tighter_than_custom_operators_at_the_same_number() {
    assert_eq!(
        execute(
            r#"
@join(left, right): left * 10 + right;
infixop 190 join;
println(1 join 2 * 3);
"#,
        ),
        "16\n"
    );
}

#[test]
fn returns_from_if_else_branches() {
    assert_eq!(
        execute(
            r#"
a: 1;
@foo(x) {
    a : 1;
    if x { return a: 2; } else { return a: 3; };
};
print(foo(1));
print(foo(0));
"#,
        ),
        "23"
    );
}

#[test]
fn evaluates_delayed_recursive_y_combinator() {
    assert_eq!(
        execute(
            r#"
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));
@fact(f): @(n): n ? (n * f(n-1)) , 1;
print(Y(fact)(5));
"#,
        ),
        "120"
    );
}

#[test]
fn nested_foreach_matches_legacy_scoping() {
    assert_eq!(
        execute(
            r#"
foreach [1, 2] as i {
    foreach [2, 3] as j { print(i); print(j); }
}
"#,
        ),
        "12132223"
    );
}

#[test]
fn supports_positional_and_destructuring_declarations() {
    assert_eq!(
        execute(
            r#"
@prints(...args) {
    foreach args as arg { print(arg); print(" "); }
}
@a, b: 1, 2;
prints(a, b);
@a, b: [3, 4];
prints(a, b);
@[a, b]: [5, 6];
prints(a, b);
"#,
        ),
        "1 2 3 4 5 6 "
    );
}

#[test]
fn delayed_values_can_preserve_references() {
    assert_eq!(
        execute(
            r#"
@refof(&var): delay var;
prefixop refof;
@a: 1;
@test(var): var: 10;
test(refof a);
println(a);
"#,
        ),
        "10\n"
    );
}

#[test]
fn supports_enums_and_match_expressions() {
    assert_eq!(
        execute(
            r#"
State: enum { Start, Running, End };
state: State.Start;
while (state != State.End) {
    println(match state {
        State.Start => { state: State.Running; return "start"; },
        State.Running => { state: State.End; return "running"; },
        => "end"
    });
}
"#,
        ),
        "start\nrunning\n"
    );
}

#[test]
fn supports_classes_instances_and_inheritance() {
    assert_eq!(
        execute(
            r#"
class Base {
    age: 10;
    __init__(self, x) { self.name: x; }
}
class (Base) Student {
    __init__(self, x) { bctors[0](self, x); }
}
stu: Student("Anole");
println(Student.age);
println(stu.age);
println(stu.name);
stu.age: 20;
println(Student.age);
println(stu.age);
"#,
        ),
        "10\n10\nAnole\n10\n20\n"
    );
}

#[test]
fn functions_expose_their_captured_scope_as_members() {
    assert_eq!(
        execute(
            r#"
@Factory: {
    @answer: 42;
    @get(): answer;
    return @{};
};
println(Factory.answer);
println(Factory.get());
"#,
        ),
        "42\n42\n"
    );
}

#[test]
fn supports_dict_lookup_and_assignment() {
    assert_eq!(
        execute(
            r#"
d: dict { "one" => 1 };
println(d["one"]);
d["two"]: 2;
println(d["two"]);
"#,
        ),
        "1\n2\n"
    );
}

#[test]
fn call_with_current_continuation_restores_the_captured_future() {
    assert_eq!(
        execute(
            r#"
Xb: @{
    println("Hi! My name is Xu Bo.");
    cont: call_with_current_continuation(Lyx);
    println("Do you love me?");
    call_with_current_continuation(cont);
}
Lyx: @(cont) {
    println("Hello! I'm Luo yuexuan.");
    cont: call_with_current_continuation(cont);
    println("Yes! I love you very very much!");
    cont(none);
}
Xb();
"#,
        ),
        "Hi! My name is Xu Bo.\nHello! I'm Luo yuexuan.\nDo you love me?\nYes! I love you very very much!\n"
    );
}

#[test]
fn continuations_can_implement_custom_exception_handling() {
    assert_eq!(
        execute(
            r#"
@Except: {
    @conts: [];
    @throw(e) {
        if conts.empty() { println(e); }
        else { @cont: conts.pop(); cont(e); }
    }
    @catch(try, cfun) {
        @e: call_with_current_continuation(@(cont) {
            conts.push(cont);
            try(@(x): x)();
        });
        if !(e is none) { cfun(e); }
    }
    @try(fun): @(f): f(fun);
    return @{};
};
@throw: Except.throw;
prefixop throw;
@try: Except.try;
prefixop try;
@catch: Except.catch;
infixop catch;
@div(a, b) {
    if b = 0 { throw "err: div 0"; }
    return a / b;
};
@div_forever(a) {
    @b: a;
    while true { div(a, b); b: b - 1; }
};
try @{ div_forever(100); }
catch @(e) { println(e); }
"#,
        ),
        "err: div 0\n"
    );
}

#[test]
fn evaluates_the_legacy_church_encoding_sample() {
    assert_eq!(
        execute(
            r#"
Zero: @(f): @(x): x;
Succ: @(n): @(f): @(x): f(n(f)(x));
Pred: @(n): @(f): @(x): n(@(g): @(h): h(g(f)))(@(u): x)(@(u): u);
Plus: @(m, n): @(f): @(x): m(f)(n(f)(x));
Mult: @(m, n): @(f): n(m(f));
Exp: @(m, n): n(m);
True: @(x, y): x;
False: @(x, y): y;
IsZero: @(n): n(@(x): False)(True);
BoolAnd: @(x, y): x(y, False);
BoolOr: @(x, y): x(True, y);
BoolNot: @(x): x(False, True);
IfThenElse: @(cond, true_expr, false_expr): cond(delay true_expr, delay false_expr);
Add2: @(x, y): IfThenElse(IsZero(x), y, delay Succ(Add2(Pred(x), y)));
Equal: @(x, y):
    IfThenElse(BoolAnd(IsZero(x), IsZero(y)), True,
        IfThenElse(BoolOr(IsZero(x), IsZero(y)), False,
            delay Equal(Pred(x), Pred(y))));
One: Succ(Zero);
Two: Plus(One, One);
Four: Mult(Two, Two);
println(Equal(Two, Plus(One, One)) = True);
println(Equal(Two, Plus(Two, Two)) = True);
println(Equal(Two, Add2(One, One)) = True);
@show(x) { println(x); return x; }
One(show)(1);
Two(show)(2);
Four(show)(4);
"#,
        ),
        "true\nfalse\ntrue\n1\n2\n2\n4\n4\n4\n4\n"
    );
}

#[test]
fn preserves_float_and_dict_object_protocols() {
    assert_eq!(
        execute(
            r#"
println(1.5);
d: dict { "one" => 1 };
println(d);
println(d.empty());
d.insert("two", 2);
println(d.size());
println(d.at("two"));
d.three: 3;
println(d["three"]);
d.erase("one");
println(d.size());
d.clear();
println(d.empty());
"#,
        ),
        "1.500000\n{ one => 1 }\nfalse\n2\n2\n3\n2\ntrue\n"
    );
}

#[test]
fn is_compares_object_identity_without_aliasing_variable_slots() {
    assert_eq!(
        execute(
            r#"
@a: 1;
@b: 1;
@c: a;
println(a is b);
println(a is c);
c: 2;
println(a);
println(c);
println(none is none);
println(true is true);
"#,
        ),
        "true\ntrue\n1\n2\ntrue\ntrue\n"
    );
}

#[test]
fn mutable_containers_share_object_state_but_not_variable_slots() {
    assert_eq!(
        execute(
            r#"
@a: [1];
@b: a;
b.push(2);
println(a);
println(a is b);
b: [];
println(a);
println(b);
@d: dict { "one" => 1 };
@e: d;
e.insert("two", 2);
println(d);
e: dict {};
println(d);
println(e);
"#,
        ),
        "[1, 2]\ntrue\n[1, 2]\n[]\n{ one => 1, two => 2 }\n{ one => 1, two => 2 }\n{ }\n"
    );
}

#[test]
fn id_observes_object_identity_and_none_keeps_legacy_rendering() {
    assert_eq!(
        execute(
            r#"
@a: 1;
@b: a;
@c: 1;
println(id(a) = id(b));
println(id(a) = id(c));
println(id(true) = id(true));
println(str(none));
println([none]);
println(none);
"#,
        ),
        "true\ntrue\ntrue\n<no definition of to_str>\n[<no definition of to_str>]\n"
    );
}

#[test]
fn rejects_truth_tests_for_objects_without_legacy_boolean_conversion() {
    let error = Interpreter::new()
        .run("if none { println(1); }", "<test>")
        .unwrap_err();
    assert_eq!(error.message, "cannot translate to bool");
}

#[test]
fn foreach_preserves_continuations_live_lists_and_iterator_protocols() {
    assert_eq!(
        execute(
            r#"
@items: [1, 2];
foreach items as item {
    @value: call_with_current_continuation(@(cont): item);
    print(value);
    if item = 2 { items.push(3); }
}
print("\n");
@Range: class {
    __init__(self) { self.i: 0; };
    __iterator__(self) { return self; };
    __has_next__(self) { return self.i < 3; };
    __next__(self) {
        @value: self.i;
        self.i: self.i + 1;
        return value;
    };
};
foreach Range() as value { print(value); }
"#,
        ),
        "12\n012"
    );
}

#[test]
fn class_constructors_run_in_the_explicit_vm_context() {
    assert_eq!(
        execute(
            r#"
@Box: class {
    __init__(self) {
        self.value: call_with_current_continuation(@(cont): 7);
    };
};
println(Box().value);
"#,
        ),
        "7\n"
    );
}
