#include <fstream>
#include "code.hpp"
#include "repl.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "argparse.hpp"
#include "moduleobject.hpp"

using namespace std;
using namespace anole;

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        replrun::run();
    }
    else
    {
        ArgumentParser parser("anole");
        parser.add_argument("file");
        parser.parse(argc, argv);

        auto path = fs::path(parser.get("file"));
        theCurrentContext = make_shared<Context>(nullptr,
            path.parent_path());
        auto filename = path.filename().string();
        if (path.extension().string() != ".anole")
        {
            cout << "anole: input file should end with .anole" << endl;
        }
        else if (!AnoleModuleObject(filename.substr(0, filename.rfind('.'))).good())
        {
            cout << "anole: cannot open file " << path << endl;
        }
    }
    return 0;
}
