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
        auto fin = ifstream(parser.get("file"));

        auto code = make_shared<Code>();
        Parser(fin).gen_statements()->codegen(*code);
        auto fout = ofstream("test.out");
        code->print(fout);

        theCurrentFrame = make_shared<Frame>(code);
        Frame::execute();
    }
    return 0;
}
