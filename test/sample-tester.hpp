#include "../src/parser.hpp"
#include "../src/context.hpp"
#include "../src/boolobject.hpp"
#include <gtest/gtest.h>
#include <cstdio>
#include <sstream>
#include <iostream>

using namespace std;
using namespace anole;

inline string execute(const string &input)
{
    istringstream ss{input};
    auto code = make_shared<Code>();
    theCurrentContext = make_shared<Context>(code);
    auto ast = Parser(ss).gen_statements();
    ast->codegen(*code);
    #ifdef _DEBUG
    code->print();
    #endif
    ostringstream out;
    auto backup = cout.rdbuf();
    cout.rdbuf(out.rdbuf());
    Context::execute();
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

Add: @(m, n): @(f): @(x): m(f)(n(f)(x));

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
Two: Add(One, One);

println(Equal(Two, Add(One, One)) = True);
println(Equal(Two, Add(Two, Two)) = True);
println(Equal(Two, Add2(One, One)) = True);)"),

// output
R"(true
false
true
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
