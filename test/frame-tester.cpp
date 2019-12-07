#include "../src/parser.hpp"
#include "../src/frame.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

TEST_CLASS(Frame)
    TEST_METHOD(SimpleRun)
        istringstream ss(R"(
a: 1
b: 2
b: a : 3
a + b
        )");
        Code code{true};
        auto frame = make_shared<Frame>();
        auto ast = Parser(ss).gen_statements();
        ast->codegen(code);
        frame->execute_code(code);
        ASSERT(*frame->pop<long>() == 6);
    TEST_END

    TEST_METHOD(SimpleFunc)
        istringstream ss(R"(
@adddd: @(a): @(b): @(c): @(d): a + b + c + d
adddd(1)(2)(3)(4)
        )");
        Code code(true);
        auto frame = make_shared<Frame>();
        auto ast = Parser(ss).gen_statements();
        ast->codegen(code);
        frame->execute_code(code);
        ASSERT(*frame->pop<long>() == 10);
    TEST_END
TEST_END
