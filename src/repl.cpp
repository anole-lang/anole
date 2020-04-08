#include <csetjmp>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "code.hpp"
#include "repl.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "noneobject.hpp"

using namespace std;

namespace
{
static string read_line(const char * str)
{
    char *temp = readline(str);
    if (!temp)
    {
        exit(0);
    }
    string line = temp;
    free(temp);
    return line;
}

static void handle_sigint(int)
{
    cout << "\b \b\b \b\nKeyboard Interrupt\n";
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
}

namespace ice_language
{
void replrun::run()
{
    signal(SIGINT, handle_sigint);
    rl_bind_key('\t', rl_insert);
    // read_history(NULL);

    cout <<
"    _____________________\n"
"   /_  ___/ _____/ _____/\n"
"    / /  / /    / /____      Version 0.0.4 \n"
" __/ /__/ /____/ /____\n"
"/______/______/______/\n"
    << endl;
    AST::interpretive() = true;

    string line;
    istringstream ss;
    Parser parser{ss};
    auto code = make_shared<Code>("<stdin>");
    theCurrentContext = make_shared<Context>(code);

    parser.set_continue_action([&ss, &parser]
    {
        auto line = read_line(".. ");
        if (!line.empty())
        {
            add_history(line.c_str());
            // write_history(line.c_str());
        }
        ss.clear();
        ss.str(line += '\n');
        parser.cont();
    });

    while (true)
    {
        do
        {
            line = read_line(">> ");
        } while (line.empty());
        add_history(line.c_str());
        // write_history(line.c_str());
        ss.clear();
        ss.str(line + '\n');
        parser.reset();

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
        catch (const CompileError &e)
        {
            cerr << e.what() << endl;
        }
        catch (const RuntimeError &e)
        {
            cerr << "\033[1mrunning at " << theCurrentContext->code()->from();
            auto &mapping = theCurrentContext->code()->mapping();
            if (mapping.count(theCurrentContext->pc()))
            {
                auto pos = mapping[theCurrentContext->pc()];
                cerr << ":" << pos.first << ":" << pos.second << ": \033[31merror:\033[0m ";
            }
            cerr << e.what() << endl;
        }
    }
}
}
