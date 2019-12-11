#include "frame.hpp"
#include "parser.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *agrv[])
{
    Code code;
    auto frame = make_shared<Frame>();
    Ptr<Parser> parser = nullptr;
    if (argc == 1)
    {
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
            parser->reset();
        }
    }
    return 0;
}
