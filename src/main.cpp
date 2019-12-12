#include "frame.hpp"
#include "parser.hpp"
#include "repl.hpp"

using namespace std;
using namespace ice_language;

int main(int argc, char *agrv[])
{
    if (argc == 1)
    {
        ReadEvalPrintLoop().run();
    }
    else
    {
        // read from file
    }
    return 0;
}
