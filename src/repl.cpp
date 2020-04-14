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

namespace anole
{
void replrun::run()
{
    signal(SIGINT, handle_sigint);
    rl_bind_key('\t', rl_insert);
    // read_history(NULL);

    cout <<
R"(    _                _
   / \   _ __   ___ | | ___
  / _ \ | '_ \ / _ \| |/ _ \
 / ___ \| | | | (_) | |  __/    Version 0.0.8
/_/   \_\_| |_|\___/|_|\___|
)"  << endl;

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
            if (dynamic_cast<ExprStmt *>(stmt.get()))
            {
                ExprList args;
                args.emplace_back(move(reinterpret_cast<ExprStmt *>(stmt.get())->expr));
                stmt = make_unique<ParenOperatorExpr>(
                    make_unique<IdentifierExpr>("println"),
                    move(args));
            }
            stmt->codegen(*code);
            Context::execute();
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
        }
    }
}
}
