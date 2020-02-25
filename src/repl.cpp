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
    theCurrentContext = make_shared<Context>(code);

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
            theCurrentContext->pc() = code->size();

            auto stmt = parser.gen_statement();
            if (dynamic_pointer_cast<ExprStmt>(stmt))
            {
                stmt = make_shared<ParenOperatorExpr>(
                    make_shared<IdentifierExpr>("println"),
                    ExprList{ reinterpret_pointer_cast<ExprStmt>(stmt)->expr }
                );
            }
            stmt->codegen(*code);

            #ifdef _DEBUG
            code->print();
            #endif

            Context::execute();
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
