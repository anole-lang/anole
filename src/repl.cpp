#include <sstream>
#include <iostream>
#include "repl.hpp"
#include "parser.hpp"
#include "noneobject.hpp"

using namespace std;

namespace ice_language
{
void replrun::run()
{
    cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.1 \n"
" __/ /__/ /____/ /____\n"
"/______/______/______/\n"
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
    auto code = make_shared<Code>();
    theCurrentFrame = make_shared<Frame>(code);

    parser.set_continue_action([&ss, &line, &parser]
    {
        cout << ".. ";
        std::getline(cin, line);
        ss.clear(); ss.str(line.empty() ? line : (line += '\n'));
        parser.cont();
    });

    while (true)
    {
        try
        {
            auto stmt = parser.gen_statement();
            stmt->codegen(*code);
            #ifdef _DEBUG
            code->print();
            #endif
            Frame::execute();
            if (dynamic_pointer_cast<ExprStmt>(stmt)
                and theCurrentFrame->top() != theNone)
            {
                cout << theCurrentFrame->pop()->to_str() << endl;
            }
        }
        catch (const exception &e)
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
