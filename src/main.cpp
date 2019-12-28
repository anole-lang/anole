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

        Code code;
        auto frame = make_shared<Frame>();
        Parser(fin).gen_statements()->codegen(code);
        frame->execute_code(code);
    }
    return 0;
}
