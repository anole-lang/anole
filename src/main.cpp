#include <fstream>
#include "code.hpp"
#include "repl.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "argparse.hpp"
#include "moduleobject.hpp"

using namespace std;
using namespace ice_language;

namespace fs = std::filesystem;

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

        auto path = parser.get("file");
        auto icei_path = path;
        icei_path.back() = 'i';

        auto code = make_shared<Code>(path);

        if (fs::is_regular_file(icei_path)
            and fs::last_write_time(icei_path) >= fs::last_write_time(path))
        {
            code->unserialize(icei_path);
        }
        else
        {
            auto fin = ifstream(path);
            if (!fin.good())
            {
                cout << "ice: can't open file '" << path << "': No such file" << endl;
            }
            Parser(fin, path).gen_statements()->codegen(*code);
            path.back() = 'r';
            code->print(path);
            path.back() = 'i';
            code->serialize(path);
        }

        theCurrentContext = make_shared<Context>(code, filesystem::path(path).parent_path());
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
