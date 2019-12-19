#include "repl.hpp"

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
