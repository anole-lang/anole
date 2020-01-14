#include <sstream>
#include <iostream>
#include "repl.hpp"
#include "parser.hpp"
#include "noneobject.hpp"

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
    AST::interpretive() = true;

    string line;
    while (line.empty())
    {
        cout << ">> ";
        std::getline(cin, line);
    }
    istringstream ss(line += '\n');

    Parser parser{ss};
    auto frame = make_shared<Frame>();

    parser.set_continue_action([&ss, &line, &parser]
    {
        cout << ".. ";
        std::getline(cin, line);
        ss.clear(); ss.str(line.empty() ? line : (line += '\n'));
        parser.cont();
    });

    size_t base = 0;

    while (true)
    {
        try
        {
            auto stmt = parser.gen_statement();
            stmt->codegen(theCode);
            frame->execute_code(theCode, base);
            base = theCode.size();
            if (dynamic_pointer_cast<ExprStmt>(stmt)
                and frame->top() != theNone)
            {
                cout << frame->pop()->to_str() << endl;
            }
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
