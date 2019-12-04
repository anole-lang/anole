#include "../src/parser.hpp"
#include "../src/frame.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

TEST_CLASS(Frame)
    TEST_METHOD(SimpleRun)
        istringstream ss(R"(1)");
        Code code{true}; Frame frame;
        auto ast = Parser(ss).gen_ast();
        ast->codegen(code);
        frame.execute_code(code);
        cout << *frame.pop<long>() << endl;
    TEST_END
TEST_END
