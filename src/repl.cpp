#include <sstream>
#include <iostream>
#include "repl.hpp"
#include "parser.hpp"

using namespace std;
using std::cout;

namespace ice_language
{
void ReadEvalPrintLoop::run()
{
    std::cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.1 \n"
" __/ /__/ /____/ /____       http://ice.jusot.com\n"
"/______/______/______/   \n"
    << std::endl;
    std::cout << ">> ";
    AST::interpret_mode() = true;

    string line;
    std::getline(cin, line);
    istringstream ss(line);

    Parser parser{ss}; Code code;
    auto frame = make_shared<Frame>();

    while (true)
    {
        try
        {
            parser.gen_statement()->codegen(code);
            frame->execute_code(code);
            cout << *frame->pop<long>() << endl;
        }
        catch (const runtime_error &e)
        {
            cerr << e.what() << endl;
        }
        catch (...)
        {
            if (!line.empty())
            {
                cout << ".. ";
                string temp;
                std::getline(cin, temp);
                line += '\n' + temp;
                ss.clear(); ss.str(line);
                parser.reset();
                continue;
            }
        }

        cout << ">> ";
        std::getline(cin, line);
        ss.clear(); ss.str(line);
        parser.reset();
    }
}
}
