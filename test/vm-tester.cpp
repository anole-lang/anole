#include <sstream>
#include <iostream>
#include "../src/codegen.cpp"
#include "../src/parser.cpp"
#include "../src/token.cpp"
#include "../src/tokenizer.cpp"
#include "../src/vm.cpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

TEST_CLASS(VM)
    TEST_METHOD(Push)
        istringstream ss(R"(1)");
        Code code; VM vm;
        auto ast = Parser(ss).gen_ast();
        ast->codegen(code);
        vm.execute_code(code);
    TEST_END
TEST_END
