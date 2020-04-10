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

        filename.back() = 'r';
        auto icer_out = ofstream(filename);
        code->print(icer_out);

        filename.back() = 'i';
        auto icei_out = ofstream(filename);
        code->serialize(icei_out);

        theCurrentContext = make_shared<Context>(code);
        try
        {
            Context::execute();
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
        }
    }
    return 0;
}
