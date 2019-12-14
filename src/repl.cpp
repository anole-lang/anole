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
    cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.1 \n"
" __/ /__/ /____/ /____       http://ice.jusot.com\n"
"/______/______/______/   \n"
    << endl;
    AST::interpret_mode() = true;

    string line;
    while (line.empty())
    {
        cout << ">> ";
        std::getline(cin, line);
    }
    istringstream ss(line += '\n');

    Parser parser{ss}; Code code;
    auto frame = make_shared<Frame>();

    parser.set_continue_action([&ss, &line, &parser]
    {
        cout << ".. ";
        std::getline(cin, line);
        ss.clear(); ss.str(line += '\n');
        parser.reset();
    });

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

        do
        {
            cout << ">> ";
            std::getline(cin, line);
        } while (line.empty());
        ss.clear(); ss.str(line += '\n');
        parser.reset();
    }
}
}
