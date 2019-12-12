#include "frame.hpp"
#include "parser.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *agrv[])
{
    // Draft of interpreter
    Code code;
    auto frame = make_shared<Frame>();
    Ptr<Parser> parser = nullptr;
    if (argc == 1)
    {
        cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.1 \n"
" __/ /__/ /____/ /____       http://ice.jusot.com\n"
"/______/______/______/   \n"
        << endl;
        cout << ">> ";
        parser = make_shared<Parser>();
        AST::interpret_mode() = true;
    }
    else
    {
        // read from file
        return 0;
    }
    while (true)
    {
        try
        {
            auto ast = parser->gen_statement();
            ast->codegen(code);
            frame->execute_code(code);
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
            cout << ">> ";
            parser->reset();
        }
    }
    return 0;
}
