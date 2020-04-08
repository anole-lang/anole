#include <fstream>
#include "code.hpp"
#include "repl.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "argparse.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        replrun::run();
    }
    else
    {
        ArgumentParser parser("ice");
        parser.add_argument("file");
        parser.parse(argc, argv);

        auto filename = parser.get("file");
        auto fin = ifstream(filename);
        if (!fin.good())
        {
            cout << "ice: can't open file '" << filename << "': No such file" << endl;
        }

        auto code = make_shared<Code>(filename);
        Parser(fin, filename).gen_statements()->codegen(*code);

        filename.back() = 'i';
        auto fout = ofstream(filename);
        code->print(fout);

        theCurrentContext = make_shared<Context>(code);
        try
        {
            Context::execute();
        }
        catch (const CompileError &e)
        {
            cerr << e.what() << endl;
        }
        catch (const RuntimeError& e)
        {
            cerr << "\033[1mrunning at " << theCurrentContext->code()->from();
            auto &mapping = theCurrentContext->code()->mapping();
            if (mapping.count(theCurrentContext->pc()))
            {
                auto pos = mapping[theCurrentContext->pc()];
                cerr << ":" << pos.first << ":" << pos.second << ": \033[31merror: ";
            }
            cerr << e.what() << "\033[0m" << endl;
        }
    }
    return 0;
}
