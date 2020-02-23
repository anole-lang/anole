#include "../src/parser.hpp"
#include "../src/frame.hpp"
#include "../src/boolobject.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

#ifdef _DEBUG
#define PRINT code->print()
#else
#define PRINT
#endif

#define PRE AST::interpretive() = true; \
            auto code = make_shared<Code>(); \
            theCurrentFrame = make_shared<Frame>(code);\
            auto ast = Parser(ss).gen_statements();\
            ast->codegen(*code);\
            PRINT; \
            Frame::execute();

TEST_CLASS(Frame)
    TEST_METHOD(SimpleRun)
        istringstream ss(R"(
a: 1;
b: 2;
b: a : 3;
a + b;
        )");
        PRE;
        ASSERT(theCurrentFrame->pop()->to_str() == "6");
    TEST_END

    TEST_METHOD(SimpleFunc)
        istringstream ss(R"(
@adddd: @(a): @(b): @(c): @(d): a + b + c + d;
adddd(1)(2)(3)(4);
        )");
        PRE;
        ASSERT(theCurrentFrame->pop()->to_str() == "10");
    TEST_END

    TEST_METHOD(SimpleIfElseStmt)
        istringstream ss(R"(
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
foo(1);
foo(0);
        )");
        PRE;
        ASSERT(theCurrentFrame->pop()->to_str() == "3");
        ASSERT(theCurrentFrame->pop()->to_str() == "2");
    TEST_END

    TEST_METHOD(Y)
        istringstream ss(R"(
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fact(f):
  @(n): n ? (n * f(n-1)) , 1;

@result: Y(fact)(5);
        )");
        PRE;
        // code.print();
        ASSERT(theCurrentFrame->pop()->to_str() == "120");
    TEST_END

    TEST_METHOD(Chunck)
        istringstream ss(R"(
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

Equal(Two, Add(One, One)) = True;
Equal(Two, Add(Two, Two)) = True;
Equal(Two, Add2(One, One)) = True;
            )");
        PRE;
        ASSERT(theCurrentFrame->pop() == theTrue);
        ASSERT(theCurrentFrame->pop() == theFalse);
        ASSERT(theCurrentFrame->pop() == theTrue);
    TEST_END
TEST_END
