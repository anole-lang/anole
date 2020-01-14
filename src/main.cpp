#include <fstream>
#include "repl.hpp"
#include "argparse.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        ReadEvalPrintLoop().run();
    }
    else
    {
        ArgumentParser parser("ice");
        parser.add_argument("file");
        parser.parse(argc, argv);
        auto fin = ifstream(parser.get("file"));

        auto frame = make_shared<Frame>();
        Parser(fin).gen_statements()->codegen(theCode);
        frame->execute_code(theCode);

        auto fout = ofstream("test.out");
        theCode.print(fout);
    }
    return 0;
}
