#include "frame.hpp"
#include "parser.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *agrv[])
{
    Code code;
    auto frame = make_shared<Frame>();
    auto ast = Parser().gen_statement();
    ast->codegen(code);
    frame->execute_code(code);
    return 0;
}
