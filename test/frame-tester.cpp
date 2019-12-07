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
a: @(a) {
    return @() {
        return a
    }
}(1)()
        )");
        Code code(true);
        auto frame = make_shared<Frame>();
        auto ast = Parser(ss).gen_statement();
        ast->codegen(code);
        frame->execute_code(code);
        ASSERT(*frame->pop<long>() == 1);
    TEST_END
TEST_END
