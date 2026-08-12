use anole::{Interpreter, Parser};

fn execute(source: &str) -> String {
    Interpreter::new().run(source, "<behavior>").unwrap()
}

// These tests define the runtime's observable behavior, including evaluation
// order and object protocol edge cases.

#[test]
fn evaluates_call_arguments_right_to_left_and_callee_last() {
    assert_eq!(
        execute(
            r#"
@callee(left, right): none;
@state: 0;
callee(state: state * 10 + 1, state: state * 10 + 2);
println(state);
state: 0;
((state: state * 10 + 3) ? callee, callee)(
    state: state * 10 + 1,
    state: state * 10 + 2
);
println(state);
"#,
        ),
        "21\n213\n"
    );
}

#[test]
fn evaluates_aggregate_parts_right_to_left() {
    assert_eq!(
        execute(
            r#"
@state: 0;
@values: [
    state: state * 10 + 1,
    state: state * 10 + 2,
    state: state * 10 + 3
];
println(state);
state: 0;
@mapping: dict {
    (state: state * 10 + 1) => (state: state * 10 + 2),
    (state: state * 10 + 3) => (state: state * 10 + 4)
};
println(state);
"#,
        ),
        "321\n4321\n"
    );
}

#[test]
fn lists_allow_adjacent_elements_without_commas() {
    assert_eq!(execute("println([1 2 3]);"), "[1, 2, 3]\n");
}

#[test]
fn evaluates_assignment_rhs_and_custom_operator_rhs_first() {
    assert_eq!(
        execute(
            r#"
@state: 0;
@boxes: [[0], [0]];
boxes[(state: state * 10 + 1) % 2][0]: state: state * 10 + 2;
println(state);
@*~*(left, right): left + right;
infixop 180 *~*;
state: 0;
@value: (state: state * 10 + 1) *~* (state: state * 10 + 2);
println(state);
println(value);
"#,
        ),
        "21\n21\n42\n"
    );
}

#[test]
fn dynamic_operator_names_stop_being_module_identifiers() {
    let error = Interpreter::new()
        .run(
            concat!(
                "@join(left, right): left + right;\n",
                "infixop 50 join;\n",
                "use join;",
            ),
            "dynamic-module.anole",
        )
        .unwrap_err();
    assert_eq!(error.message, "expect a module here");
    assert_eq!(
        error.location.unwrap(),
        anole::Location { line: 3, column: 4 }
    );
}

#[test]
fn dynamic_operator_names_are_not_class_identifiers() {
    let error = Interpreter::new()
        .run(
            concat!(
                "@join(left, right): left + right;\n",
                "infixop 50 join;\n",
                "class join {}",
            ),
            "dynamic-class.anole",
        )
        .unwrap_err();
    assert_eq!(error.message, "expected '{'");
    assert_eq!(
        error.location.unwrap(),
        anole::Location { line: 3, column: 6 }
    );
}

#[test]
fn operator_declarations_do_not_reclassify_the_already_lexed_lookahead() {
    assert_eq!(
        execute(concat!(
            "@join(left, right): left + right;",
            "infixop 50 join;",
            "join; println(\"infix\");",
            "@negate(value): 0 - value;",
            "prefixop negate;",
            "negate; println(\"prefix\");",
        )),
        "infix\nprefix\n"
    );
}

#[test]
fn infix_operator_tokens_cannot_start_a_later_statement() {
    let error = Interpreter::new()
        .run(
            concat!(
                "@join(left, right): left + right;",
                "infixop 50 join;",
                "println(0); join 1;",
            ),
            "dynamic-statement.anole",
        )
        .unwrap_err();
    assert_eq!(error.message, "wrong token here");
}

#[test]
fn repeated_infix_declarations_inside_one_statement_keep_every_layer() {
    assert_eq!(
        execute(concat!(
            "@join(left, right): left * 10 + right;",
            "if true { infixop 50 join; infixop 190 join; }",
            "println(1 + 2 join 3);",
        )),
        "24\n"
    );
}

#[test]
fn assignment_binds_inside_a_pending_binary_rhs() {
    assert_eq!(
        execute(
            r#"
@left: 1;
@right: 2;
println(left + right: 3);
println(left);
println(right);
"#,
        ),
        "4\n1\n3\n"
    );
}

#[test]
fn discards_a_statement_leading_at_before_parsing_the_expression() {
    assert_eq!(
        execute(
            r#"
@{ println("block"); };
@(println("paren"));
@value: @(): 42;
println(value());
"#,
        ),
        "block\nparen\n42\n"
    );
}

#[test]
fn control_flow_declarations_share_the_surrounding_scope() {
    assert_eq!(
        execute(
            r#"
@value: 1;
if true { @value: 2; }
println(value);
@count: 0;
while count < 1 {
    @inside: 42;
    count: count + 1;
}

println(inside);
foreach [41, 42] as item {}
println(item);
@functions: [];
foreach [1, 2] as captured {
    functions.push(@(): captured);
}
println(functions[0]());
println(functions[1]());
"#,
        ),
        "2\n42\n42\n2\n2\n"
    );
}

#[test]
fn local_redeclarations_rebind_the_existing_variable_slot() {
    assert_eq!(
        execute(
            r#"
@value: 1;
@&alias: value;
@value: 2;
println(alias);
println(value);

@value;
println(alias is none);
println(value is none);
"#,
        ),
        "2\n2\ntrue\ntrue\n"
    );
}

#[test]
fn variadic_parameters_rebind_an_existing_parameter_slot() {
    assert_eq!(
        execute(
            r#"
@capture(&slot, ...slot): slot;
@value: 1;
println(capture(value, 2, 3));
println(value);

@capture_refs(&slot, ...&slot): slot;
@other: 4;
println(capture_refs(other, value));
println(other);
"#,
        ),
        "[2, 3]\n[2, 3]\n[[2, 3]]\n[[2, 3]]\n"
    );
}

#[test]
fn default_arguments_can_see_prior_parameters() {
    assert_eq!(
        execute(
            r#"
@pick(value, fallback: value): fallback;
println(pick(42));
"#,
        ),
        "42\n"
    );
}

#[test]
fn parameter_scanning_skips_nested_lambda_default_bytecode() {
    assert_eq!(
        execute(
            r#"
@defaults(callback: (@(value) { @lazy: delay value + 1; return lazy; }),
          transform: (@(value): value * 2)):
    [callback(4), transform(4)];
println(defaults());
println(defaults((@(value): value * 2), (@(value): value + 5)));
"#,
        ),
        "[5, 8]\n[8, 9]\n"
    );
}

#[test]
fn continuations_work_in_composite_expression_contexts() {
    assert_eq!(
        execute(include_str!("fixtures/continuation_vm_contexts.anole")),
        "42\n43\n44\n"
    );
}

#[test]
fn continuation_scope_copy_is_visible_in_class_bodies() {
    let error = Interpreter::new()
        .run(
            r#"
class Answer {
    value: call_with_current_continuation(@(continuation): continuation(43));
}
println(Answer.value);
"#,
            "<behavior>",
        )
        .unwrap_err();
    assert_eq!(error.message, "no member named value");
}

#[test]
fn continuation_resumption_copies_the_context_scope() {
    assert_eq!(
        execute(include_str!("fixtures/continuation_scope_snapshot.anole")),
        "0\n0\n"
    );
}

#[test]
fn continuation_resumption_restores_the_operand_stack() {
    assert_eq!(
        execute(include_str!("fixtures/continuation_stack.anole")),
        "12\n14\n16\n"
    );
}

#[test]
fn continuations_require_exactly_one_argument() {
    for source in [
        "call_with_current_continuation(@(continuation): continuation());",
        "call_with_current_continuation(@(continuation): continuation(1, 2));",
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(error.message, "continuation need a argument");
    }
}

#[test]
fn continuation_results_use_a_fresh_variable_slot() {
    assert_eq!(
        execute(include_str!("fixtures/continuation_argument_slot.anole")),
        "1\n"
    );
}

#[test]
fn callcc_rejects_non_callable_arguments_with_its_legacy_error() {
    let error = Interpreter::new()
        .run("call_with_current_continuation(1);", "<behavior>")
        .unwrap_err();
    assert_eq!(error.message, "err type as the argument for call/cc");
}

#[test]
fn callcc_binds_only_the_first_parameter_then_uses_the_shared_stack() {
    assert_eq!(
        execute("println(call_with_current_continuation(@(continuation, extra): extra, 42));",),
        "42\n"
    );
}

#[test]
fn callcc_can_resume_a_continuation_with_the_new_current_continuation() {
    assert_eq!(
        execute(
            r#"
@saved: none;
@phase: 0;
@value: call_with_current_continuation(@(continuation) {
    saved: continuation;
    return 1;
});
println(type(value));
if phase = 0 {
    phase: 1;
    call_with_current_continuation(saved);
}
"#,
        ),
        "integer\ncont\n"
    );
}

#[test]
fn callcc_reports_bad_any_cast_for_non_store_first_parameters() {
    for source in [
        "call_with_current_continuation(@(continuation: none): continuation);",
        "call_with_current_continuation(@(): none);",
        "call_with_current_continuation(@(...continuations): continuations);",
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(error.to_string(), "bad any_cast", "source: {source}");
    }
}

#[test]
fn unbound_slots_support_forward_references_but_fail_when_consumed() {
    assert_eq!(
        execute(
            r#"
@identity(&value): @(): value;
@recursive: identity(recursive);
println(type(recursive()));
"#,
        ),
        "func\n"
    );

    let error = Interpreter::new()
        .run("println(missing);", "<behavior>")
        .unwrap_err();
    assert_eq!(
        error.message,
        "var named missing doesn't reference to any object"
    );

    let error = Interpreter::new()
        .run(
            "@target: 1; target: missing; println(\"after\");",
            "<behavior>",
        )
        .unwrap_err();
    assert_eq!(
        error.message,
        "var named missing doesn't reference to any object"
    );

    for source in [
        "[missing];",
        "dict { missing => 1 };",
        "1 + missing;",
        "missing is none;",
        "@[value]: missing;",
        "class (missing) Invalid {}",
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(
            error.message, "var named missing doesn't reference to any object",
            "source: {source}"
        );
    }
}

#[test]
fn unbound_slot_errors_use_the_most_recent_loaded_name() {
    for (source, expected_name) in [
        ("@&alias: missing; println(alias);", "alias"),
        (
            "@mapping: dict {}; @&alias: mapping.missing; println(alias);",
            "alias",
        ),
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(
            error.message,
            format!("var named {expected_name} doesn't reference to any object"),
            "source: {source}",
        );
    }
}

#[test]
fn dicts_preserve_object_keys_mutation_lookup_and_rendering() {
    assert_eq!(
        execute(
            r#"
@function(): none;
@functions: dict { function => 7 };
println(functions[function]);
println(functions);

@key: [1];
@mutable: dict { key => 8 };
key.push(2);
println(mutable[key]);

@left: dict { "x" => 1 };
@right: dict { "x" => 2 };
@outer: dict { left => 10, right => 20 };
println(outer.size());
println(outer[left]);
println(outer[right]);

println(dict {});
println(dict { none => 1, 2.5 => 2, "a" => 3 });
"#,
        ),
        concat!(
            "7\n",
            "{ <function> => 7 }\n",
            "8\n",
            "2\n",
            "10\n",
            "20\n",
            "{ }\n",
            "{ 2.500000 => 2, <no definition of to_str> => 1, a => 3 }\n",
        )
    );
}

#[test]
fn mutable_dict_keys_do_not_rebalance_the_existing_tree() {
    assert_eq!(
        execute(
            r#"
@low: [1];
@high: [3];
@mapping: dict { low => "low", high => "high" };
low[0]: 4;
println(mapping);
"#,
        ),
        "{ [4] => low, [3] => high }\n"
    );
}

#[test]
fn dicts_replace_entries_with_duplicate_rendered_keys() {
    assert_eq!(
        execute(
            r#"
@mapping: dict {
    "x" => 1,
    "x" => 2,
    [1] => 3,
    [1] => 4,
    1.0000001 => 5,
    1.0000002 => 6
};
println(mapping.size());
println(mapping["x"]);
println(mapping[[1]]);
println(mapping[1.0000002]);
println(mapping);
"#,
        ),
        "3\n2\n4\n6\n{ 1.000000 => 6, [1] => 4, x => 2 }\n"
    );
}

#[test]
fn dict_at_copies_the_slot_but_indexing_aliases_it() {
    assert_eq!(
        execute(
            r#"
@mapping: dict { "value" => 1 };
@&through_at: mapping.at("value");
through_at: 2;
println(mapping.value);
@&through_index: mapping["value"];
through_index: 3;
println(mapping.value);
"#,
        ),
        "1\n3\n"
    );
}

#[test]
fn enums_and_list_iterators_have_distinct_object_protocols() {
    assert_eq!(
        execute(
            r#"
@State: enum { Start, Running: 4, End };
println(type(State));
println(State.Start);
println(State.Running);
println(State.End);

@iterator: [10, 20].__iterator__();
println(type(iterator));
println(iterator.__has_next__());
println(iterator.__next__());
println(iterator.__has_next__());
println(iterator.__next__());
println(iterator.__has_next__());
"#,
        ),
        "enum\n0\n4\n5\nlistiterator\ntrue\n10\ntrue\n20\nfalse\n"
    );
}

#[test]
fn native_object_methods_are_builtin_functions() {
    assert_eq!(
        execute(
            r#"
@items: [1];
@mapping: dict {};
println(type(items.push));
println(str(items.push));
println(type("text".size));
println(type(mapping.insert));
"#,
        ),
        "builtinfunc\n<builtin-function>\nbuiltinfunc\nbuiltinfunc\n"
    );
}

#[test]
fn bound_methods_capture_the_current_object_not_the_variable_slot() {
    assert_eq!(
        execute(
            r#"
@number: 1;
@to_string: number.to_str;
number: 2;
println(to_string());

@items: [1];
@old_items: items;
@push: items.push;
items: [2];
push(3);
println(old_items);
println(items);

class Box {
    value: 0;
    get(self): self.value;
    __init__(self, value) { self.value: value; }
}
@box: Box(4);
@get: box.get;
box: Box(5);
println(get());
println(box.get());
"#,
        ),
        "1\n[1, 3]\n[2]\n4\n5\n"
    );
}

#[test]
fn native_object_methods_ignore_extra_arguments() {
    assert_eq!(
        execute(
            r#"
@items: [1];
println(items.size(90, 91));
items.push(2, 3);
println(items);
println(items.front(90));
println(items.back(90));

@iterator: items.__iterator__(90);
println(iterator.__has_next__(90));
println(iterator.__next__(90));

@mapping: dict {};
mapping.insert("answer", 42, 90);
println(mapping.at("answer", 90));
mapping.erase("answer", 90);
println(mapping.empty(90));

println("123tail".to_int(90));
println("abc".size(90));
println((42).to_str(90));
"#,
        ),
        concat!(
            "1\n", "[1, 2]\n", "1\n", "2\n", "true\n", "1\n", "42\n", "true\n", "123\n", "3\n",
            "42\n",
        )
    );
}

#[test]
fn ignored_builtin_and_native_method_arguments_are_not_dereferenced() {
    assert_eq!(
        execute(
            r#"
println(42, missing);
println(str(7, missing));
println(type(7, missing));
println(id(none, missing) = id(none));
println(type(time(missing)));
println(eval("6 * 7", missing));
println(call_with_current_continuation(@(continuation): 9, missing));

@items: [1];
println(items.size(missing));
items.push(2, missing);
println(items);
println(items.front(missing));
@mapping: dict {};
mapping.insert("answer", 42, missing);
println(mapping.at("answer", missing));
"#,
        ),
        "42\n7\ninteger\ntrue\ninteger\n42\n9\n1\n[1, 2]\n1\n42\n"
    );
}

#[test]
fn id_values_are_not_reused_for_consecutive_temporary_objects() {
    assert_eq!(
        execute(
            r#"
println(id([1]) = id([2]));
println(id(dict { "left" => 1 }) = id(dict { "right" => 2 }));
println(id(@(): none) = id(@(): none));
@seen: dict {};
@index: 0;
while index < 256 {
    seen.insert(id([index]), true);
    index: index + 1;
}
println(seen.size());
"#,
        ),
        "false\nfalse\nfalse\n256\n"
    );
}

#[test]
fn dict_pointer_keys_preserve_singleton_order() {
    assert_eq!(
        execute("println(dict { none => \"none\", true => \"true\", false => \"false\" });"),
        "{ true => true, <no definition of to_str> => none, false => false }\n"
    );
}

#[test]
fn builtin_lookups_use_fresh_variable_slots() {
    assert_eq!(
        execute(
            r#"
@&saved_print: print;
print: 1;
saved_print("first");
@&alias: println;
alias: 2;
println("second");
println(type(print));
println(print is print);
"#,
        ),
        "firstsecond\nbuiltinfunc\ntrue\n"
    );
}

#[test]
fn list_iterators_keep_their_node_after_removing_an_earlier_item() {
    assert_eq!(
        execute(include_str!("fixtures/list_iterator_mutation.anole")),
        "1\n2\n"
    );
}

#[test]
fn list_iterators_do_not_reenter_after_reaching_end() {
    assert_eq!(
        execute(
            r#"
@items: [1, 2];
@iterator: items.__iterator__();
iterator.__next__();
iterator.__next__();
items.push(3);
println(iterator.__has_next__());
"#,
        ),
        "false\n"
    );
}

#[test]
fn functions_have_a_member_scope_in_front_of_their_captured_scope() {
    assert_eq!(
        execute(
            r#"
@function(): hidden;
function.hidden: 42;
println(function());
"#,
        ),
        "42\n"
    );

    let error = Interpreter::new()
        .run(
            include_str!("fixtures/function_missing_member.anole"),
            "function_missing_member.anole",
        )
        .unwrap_err();
    assert_eq!(
        error.message,
        "var named missing doesn't reference to any object"
    );
}

#[test]
fn eval_runs_as_a_child_vm_context() {
    assert_eq!(
        execute(include_str!("fixtures/eval_continuation.anole")),
        "10\n30\n"
    );
}

#[test]
fn strings_compare_convert_and_index_as_byte_sequences() {
    assert_eq!(
        execute(
            r#"
println(1 > 1);
println(1 >= 1);
println(2 > 1);
println(2 >= 1);
@integer_left: 1;
@integer_right: 1;
println(integer_left > integer_right);
println(integer_left >= integer_right);
@float_left: 1.0;
@float_right: 1.0;
println(float_left > float_right);
println(float_left >= float_right);
println("b" > "b");
println("b" >= "b");
println("b" > "a");
println("b" >= "a");
println("a" < 1);
println("a" <= 1);
println("123suffix".to_int());
println("  -42tail".to_int());
println("é".size());
println("é"[0].size());
println("é"[1].size());
"#,
        ),
        concat!(
            "false\ntrue\ntrue\ntrue\n",
            "true\nfalse\ntrue\nfalse\n",
            "true\nfalse\ntrue\ntrue\n",
            "true\ntrue\n",
            "123\n-42\n",
            "2\n1\n1\n",
        )
    );

    let error = Interpreter::new()
        .run("\"value\".to_int();", "<behavior>")
        .unwrap_err();
    assert_eq!(error.message, "stoll");
}

#[test]
fn formats_runtime_nan_values_with_the_legacy_spelling() {
    assert_eq!(
        execute(
            r#"
println(0.0 / 0.0);
println(dict { (0.0 / 0.0) => 1 });
"#,
        ),
        "-nan\n{ -nan => 1 }\n"
    );
}

#[test]
fn greater_comparisons_dispatch_through_the_right_operand() {
    assert_eq!(
        execute(
            r#"
println(1 > "a");
println(1 >= "a");
println(none > "a");
println(none >= "a");
println([1] > "a");
println([1] >= "a");
"#,
        ),
        "true\ntrue\ntrue\ntrue\ntrue\ntrue\n"
    );

    let error = Interpreter::new()
        .run(r#"println("a" > 1);"#, "<behavior>")
        .unwrap_err();
    assert_eq!(error.message, "no match method");
}

#[test]
fn destructuring_requires_exact_list_lengths() {
    for (source, expected) in [
        ("@[left, right]: [1];", "expect 2 but given 1"),
        ("@[only]: [1, 2];", "expect 1 but given 2"),
        ("@left, right: [1];", "expect 2 but given 1"),
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
}

#[test]
fn match_uses_the_left_values_comparison_protocol() {
    let error = Interpreter::new()
        .run(r#"println(match 1 { "1" => 10, => 20 });"#, "<behavior>")
        .unwrap_err();
    assert_eq!(error.message, "no match method");

    assert_eq!(execute(r#"println(match "1" { 1 => 10, => 20 });"#), "20\n");
}

#[test]
fn match_stops_evaluating_keys_after_the_first_match() {
    assert_eq!(
        execute(
            r#"
@probe(value) { println(value); return value; }
println(match 2 { probe(1), probe(2) => 20, probe(3) => 30, => 40 });
"#,
        ),
        "1\n2\n20\n"
    );
}

#[test]
fn reports_precise_class_unpack_and_arity_errors() {
    for (source, expected) in [
        (
            "class (1) Invalid {}",
            "each base of one class must be one class",
        ),
        ("@left, right: 1;", "expect list expr"),
        ("@[left, right]: 1;", "expect list expr"),
        (
            "@collect(...items): items; @one: 1; collect(one...);",
            "expect list expr",
        ),
        (
            "@pair(left, right): left; pair(1);",
            "missing the parameter named 'right'",
        ),
        (
            "@pair(left, right): left; pair(1, 2, 3);",
            "function takes 2 arguments but 3 were given",
        ),
        (
            "class Plain {} Plain(1);",
            "only default ctor but given non-zero arguments",
        ),
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
}

#[test]
fn reports_object_protocol_errors_for_builtin_operators() {
    for (source, expected) in [
        ("1 + 1.0;", "no match method"),
        ("[1] + 1;", "expected"),
        ("none + 1;", "no add method"),
        ("1 - 1.0;", "no match method"),
        ("none - 1;", "no sub method"),
        ("1 % 1.0;", "no match method"),
        ("1.0 % 1;", "no mod method"),
        ("1 << 1.0;", "no match method"),
        ("none << 1;", "no bls method"),
        ("-none;", "no neg method"),
        ("~none;", "no bneg method"),
        ("1[0];", "not support index"),
        ("1();", "failed call with the given non-function"),
    ] {
        let error = Interpreter::new().run(source, "<behavior>").unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
}

#[test]
fn class_inheritance_snapshots_members_and_tracks_every_base_constructor() {
    assert_eq!(
        execute(
            r#"
class Base {
    value: 1;
    __init__(self): none;
}
class (Base) Derived {}
Base.value: 9;
println(Derived.value);
println(Derived.bctors.size());

class Left { selected: 10; }
class Right { selected: 20; }
class (Left, Right) Combined {}
Left.selected: 11;
Right.selected: 21;
println(Combined.selected);
println(Combined.bctors.size());
"#,
        ),
        "1\n1\n20\n2\n"
    );
}

#[test]
fn class_calls_return_the_constructor_result() {
    assert_eq!(
        execute(
            r#"
class Answer {
    __init__(self) { return 42; }
}
println(Answer());
"#,
        ),
        "42\n"
    );
}

#[test]
fn shorthand_constructors_return_their_explicit_expression() {
    assert_eq!(
        execute(
            r#"
class Kind { __init__(self): none; }
@value: Kind();
println(type(value));
println(value is none);
println(dict { none => 1, value => 2 }.size());
"#,
        ),
        "none\ntrue\n1\n"
    );
}

#[test]
fn default_constructor_leaves_an_existing_operand_as_the_call_result() {
    assert_eq!(
        execute(concat!(
            "class Empty {}",
            "str(1, 77);",
            "println(Empty());",
        )),
        "77\n"
    );
}

#[test]
fn print_builtins_read_and_preserve_the_shared_operand_stack() {
    assert_eq!(
        execute(concat!(
            "str(1, 66); println();",
            "println(none);",
            "class Empty {} println(Empty());",
        )),
        "66\n"
    );
}

#[test]
fn classes_reject_assigning_undeclared_class_members() {
    let error = Interpreter::new()
        .run("class Empty {} Empty.new_member: 1;", "<behavior>")
        .unwrap_err();
    assert_eq!(error.message, "no member named new_member");
}

#[test]
fn reuses_scalar_literal_objects_within_one_code_unit() {
    assert_eq!(
        execute(
            r#"
@integer_a: 1;
@integer_b: 1;
@float_a: 2.5;
@float_b: 2.5;
@string_a: "value";
@string_b: "value";
println(integer_a is integer_b);
println(float_a is float_b);
println(string_a is string_b);
println(id(integer_a) = id(integer_b));
println(id(float_a) = id(float_b));
println(id(string_a) = id(string_b));
"#,
        ),
        "true\ntrue\ntrue\ntrue\ntrue\ntrue\n"
    );
}

#[test]
fn constant_identity_follows_code_unit_boundaries() {
    assert_eq!(
        execute(
            r#"
@literal(): 7;
@from_first_call: literal();
@from_second_call: literal();
@from_first_eval: eval("7");
@from_second_eval: eval("7");
println(from_first_call is from_second_call);
println(from_first_eval is from_second_eval);
"#,
        ),
        "true\nfalse\n"
    );
}

#[test]
fn explicitly_rejects_ambiguous_declarations_and_class_members() {
    for (source, expected) in [
        ("@left, right;", "expect expressions"),
        (
            "@&reference;",
            "reference should be binded with other variable",
        ),
        ("class C { @value: 1; }", "expect an identifier here"),
        (
            "class C { left, right: 1, 2; }",
            "not support multi-declaration in class",
        ),
        (
            "class C { __init__: 1; }",
            "__init__ must be with function body",
        ),
        (
            "class C { __init__(): none; }",
            "method need at least 1 parameter",
        ),
        (
            "println(match 1 { => 1, => 2 });",
            "redefinition of else-expr of match-expr",
        ),
        ("elif true {}", "wrong token here"),
        ("@collect(...items: 1): items;", "expected ')' here"),
        ("println(match 1 { 1, => 10 });", "expected an expr here"),
        ("println(match 1 { 1 => 10 2 => 20 });", "expected '}'"),
        (
            "use \"module.anole\" as module from parent;",
            "unexpected from because there is at least one module denoted by its path",
        ),
        (
            "infixop 50 join; infixop 180 join;",
            "expected an identifier here",
        ),
        (
            "prefixop twist; infixop 50 twist;",
            "expected an identifier here",
        ),
        ("infixop 50 join; println(join);", "expected an expr here"),
        (
            concat!(
                "@join(left, right): left + right;",
                "if false { infixop 50 join; }",
                "println(1 join 2);",
            ),
            "expected ')'",
        ),
        (
            concat!(
                "@join(left, right): left + right;",
                "if true { infixop 50 join; println(1 join 2); }",
            ),
            "expected ')'",
        ),
        ("@function() { return }", "expected an expr here"),
    ] {
        let error = Parser::new(source, "<behavior>")
            .unwrap()
            .parse()
            .unwrap_err();
        assert_eq!(error.message, expected, "source: {source}");
    }
}
