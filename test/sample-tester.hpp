#include "../src/anole.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <sstream>
#include <iostream>

using namespace std;
using namespace anole;

inline String execute(const String &input)
{
    ostringstream out;
    auto backup = cout.rdbuf();
    cout.rdbuf(out.rdbuf());

    istringstream ss{input};
    auto code = make_shared<Code>();
    Context::current() = std::make_shared<Context>(code);
    Parser parser{ss};
    while (auto ast = parser.gen_statement())
    {
        ast->codegen(*code);
        Context::execute();
    }
    #ifdef _DEBUG
    code->print();
    #endif
    cout.rdbuf(backup);
    return out.str();
}

TEST(Sample, SimpleRun)
{
    ASSERT_EQ(execute(
// input
R"(
a: 1;
b: 2;
b: a : 3;
print(a + b);)"),

// output
"6");
}

TEST(Sample, SimpleFunction)
{
    ASSERT_EQ(execute(
// input
R"(
@adddd: @(a): @(b): @(c): @(d): a + b + c + d;
print(adddd(1)(2)(3)(4));)"),

// output
"10");
}

TEST(Sample, VariadicArguments)
{
    ASSERT_EQ(execute(
// input
R"(
@foo(a, b, ...c) {
    return c;
}

@l: [2, 3, 4, 5, 6];

println(foo(1, l..., 7));)"),

// output
R"([3, 4, 5, 6, 7]
)");
}

TEST(Sample, SimpleIfElseStmt)
{
    ASSERT_EQ(execute(
// input
R"(
a: 1;
@foo(x)
{
    a : 1;
    if x
    {
        return a: 2;
    }
    else
    {
        return a: 3;
    };
};
print(foo(1));
print(foo(0));)"),

// output
"23");
}

TEST(Sample, Y)
{
    ASSERT_EQ(execute(
// input
R"(
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fact(f):
  @(n): n ? (n * f(n-1)) , 1;

print(Y(fact)(5));)"),

// output
"120");
}

TEST(Sample, Chunch)
{
    ASSERT_EQ(execute(
// input
R"(
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

IfThenElse:
    @(cond, true_expr, false_expr):
        cond(delay true_expr, delay false_expr);

Add2: @(x, y): IfThenElse(IsZero(x), y, delay Succ(Add2(Pred(x), y)));

Equal: @(x, y):
    IfThenElse(BoolAnd(IsZero(x), IsZero(y)),
        True,
        IfThenElse(BoolOr(IsZero(x), IsZero(y)),
            False,
            delay Equal(Pred(x), Pred(y))));

One: Succ(Zero);
Two: Plus(One, One);
Four: Mult(Two, Two);

println(Equal(Two, Plus(One, One)) = True);
println(Equal(Two, Plus(Two, Two)) = True);
println(Equal(Two, Add2(One, One)) = True);

@show(x) {
    println(x);
    return x;
}

One(show)(1);
Two(show)(2);
Four(show)(4);)"),

// output
R"(true
false
true
1
2
2
4
4
4
4
)");
}

TEST(Sample, Continuation)
{
    ASSERT_EQ(execute(
// input
R"code(
Xb: {
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
        )code"),

// output
R"(Hi! My name is Xu Bo.
Hello! I'm Luo yuexuan.
Do you love me?
Yes! I love you very very much!
)");
}

TEST(Sample, CostumTryCatch)
{
    ASSERT_EQ(execute(
// input
R"(
@Except: {
    @conts: [];

    @throw(e) {
        if conts.empty() {
            println(e);
            exit();
        } else {
            @cont: conts.pop();
            cont(e);
        }
    }

    @catch(try, cfun) {
        @e: call_with_current_continuation(@(cont) {
            conts.push(cont);
            try(@(x): x)();
        });
        if !(e is none), cfun(e);
    }

    @try(fun): @(f): f(fun);

    return {};
}();

@throw: Except.throw;
prefixop throw;

@try: Except.try;
prefixop try;

@catch: Except.catch;
infixop catch;

@div(a, b) {
    if b = 0, throw "err: div 0";
    return a / b;
};

@div_forever(a) {
    @b: a;
    while true {
        div(a, b);
        b: b - 1;
    }
};

try {
    div_forever(100);
}
catch @(e) {
    println(e);
})"),

// output
R"(err: div 0
)");
}

TEST(Sample, CustomOp)
{
    ASSERT_EQ(execute(
// input
R"(
@*=*(lhs, rhs): lhs + rhs;

@*-*(lhs, rhs), return lhs + rhs;

@*^*(lhs, rhs) {
    return lhs + rhs;
}

infixop 50  *=*;
infixop 100 *-*;
infixop 200 *^*;

println(2 * 3 *=* 4 * 5);
println(2 * 3 *-* 4 * 5);
println(2 * 3 *^* 4 * 5);

@refof(&var): delay var;
prefixop refof;

@a: 1;
@test(var): var: 10;
test(refof a);
println(a);)"),

// output
R"(26
26
70
10
)");
}

TEST(Sample, DefaultArgument)
{
    ASSERT_EQ(execute(
// input
R"(
foo: @(m, n: n) {
    return m + n;
};

n: 1;
println(foo(1));

n: 2;
println(foo(1));

foo: @(x, func: @(x): x + y) {
    return func(x);
};

y: 10;
println(foo(10));

println(foo(10, @(x): x + 1));)"),

// output
R"(2
3
20
11
)");
}

TEST(Sample, SimpleEnum)
{
    ASSERT_EQ(execute(
// input
R"(
State: enum {
    Start,
    Running,
    End
};

state: State.Start;
while (state != State.End) {
    println(match state {
        State.Start => {
            state: State.Running;
            return "start";
        }(),
        State.Running => {
            state: State.End;
            return "running"
        }(),
    } else "end");
})"),

// output
R"(start
running
)");
}
