#include <fstream>
#include "repl.hpp"
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

        auto code = make_shared<Code>();
        Parser(fin).gen_statements()->codegen(*code);

        filename = filename.substr(0, filename.size() - 1);
        auto fout = ofstream(filename);
        code->print(fout);

        theCurrentContext = make_shared<Context>(code);
        Context::execute();
    }
    return 0;
}
