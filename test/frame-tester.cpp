#include "../src/parser.hpp"
#include "../src/frame.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

#define PRE Code code{true};\
            auto frame = make_shared<Frame>();\
            auto ast = Parser(ss).gen_statements();\
            ast->codegen(code);\
            frame->execute_code(code);

TEST_CLASS(Frame)
    TEST_METHOD(SimpleRun)
        istringstream ss(R"(
a: 1
b: 2
b: a : 3
a + b
        )");
        PRE;
        ASSERT(*frame->pop<long>() == 6);
    TEST_END

    TEST_METHOD(SimpleFunc)
        istringstream ss(R"(
@adddd: @(a): @(b): @(c): @(d): a + b + c + d
adddd(1)(2)(3)(4)
        )");
        PRE;
        ASSERT(*frame->pop<long>() == 10);
    TEST_END

    TEST_METHOD(SimpleIfElseStmt)
        istringstream ss(R"(
a: 1
@foo(x)
{
    a : 1
    if x
    {
        return a: 2
    }
    else
    {
        return a: 3
    }
}
foo(1)
foo(0)
        )");
        PRE;
        ASSERT(*frame->pop<long>() == 3);
        ASSERT(*frame->pop<long>() == 2);
    TEST_END
TEST_END
