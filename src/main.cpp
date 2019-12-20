#include <fstream>
#include "repl.hpp"

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
        // ...
    }
    return 0;
}
