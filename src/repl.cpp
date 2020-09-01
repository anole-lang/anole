#include "repl.hpp"
#include "anole.hpp"

#ifdef __linux__
#include <readline/readline.h>
#include <readline/history.h>

#include <setjmp.h>
#include <unistd.h>
#include <signal.h>
#else
#error "only support linux"
#endif

#include <cstdio>
#include <sstream>
#include <iostream>

using namespace std;
using namespace anole;

namespace
{
sigjmp_buf lc_env;

String read_line(const char * str)
{
    char *temp = readline(str);
    if (!temp)
    {
        exit(0);
    }
    String line = temp;
    free(temp);
    return line;
}

void handle_sigint(int)
{
    cout << "\b \b\b \b\nKeyboard Interrupt\n";
    rl_on_new_line();
    rl_replace_line("", 0);
    siglongjmp(lc_env, 1);
}
}

namespace anole
{
void replrun::run()
{
    signal(SIGINT, handle_sigint);
    rl_bind_key('\t', rl_insert);
    // read_history(NULL);

    printf("\
    _                _" R"(
   / \   _ __   ___ | | ___
  / _ \ | '_ \ / _ \| |/ _ \
 / ___ \| | | | (_) | |  __/   %s
/_/   \_\_| |_|\___/|_|\___|)""\n\n", Version::literal);

    AST::interpretive() = true;

    String line;
    istringstream ss;
    Parser parser{ss};
    auto code = make_shared<Code>("<stdin>");
    Context::current() = make_shared<Context>(code);

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

    sigsetjmp(lc_env, 1);
    for(;;) try
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

        Context::current()->pc() = code->size();

        auto stmt = parser.gen_statement();
        if (stmt->is_expr_stmt())
        {
            ArgumentList args;
            args.emplace_back(move(reinterpret_cast<ExprStmt *>(stmt.get())->expr), false);
            stmt = make_unique<ParenOperatorExpr>(
                make_unique<IdentifierExpr>("println"),
                move(args)
            );
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
