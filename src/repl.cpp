#include "repl.hpp"
#include "parser.hpp"

using namespace std;

namespace ice_language
{
// Draft
void ReadEvalPrintLoop::run()
{
    AST::interpret_mode() = true;
    Code code;
    auto frame = make_shared<Frame>();
    // should be in interpret class
    std::cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.1 \n"
" __/ /__/ /____/ /____       http://ice.jusot.com\n"
"/______/______/______/   \n"
    << std::endl;
    std::cout << ">> ";
    Parser parser;
    while (true)
    {
        try
        {
            auto ast = parser.gen_statement();
            ast->codegen(code);
            frame->execute_code(code);
            cout << *frame->pop<long>() << endl;
            cout << ">> ";
            parser.reset();
        }
        catch (const runtime_error &e)
        {
            cerr << e.what() << endl;
        }
    }
}
}
