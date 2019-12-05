#include "../src/parser.hpp"
#include "../src/frame.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

TEST_CLASS(Frame)
    TEST_METHOD(SimpleRun)
        istringstream ss(R"(
@a: 1
@b: 2
a + b
@b: 3
a + b
        )");
        Code code{true}; Frame frame;
        auto ast = Parser(ss).gen_statements();
        ast->codegen(code);
        frame.execute_code(code);
        ASSERT(*frame.pop<long>() == 4);
    TEST_END
TEST_END
