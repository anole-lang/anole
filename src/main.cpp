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

        auto path = parser.get("file");
        auto ir_path = path + ".ir";

        auto code = make_shared<Code>(path);

        if (fs::is_regular_file(ir_path)
            and fs::last_write_time(ir_path) >= fs::last_write_time(path))
        {
            code->unserialize(ir_path);
        }
        else
        {
            auto fin = ifstream(path);
            if (!fin.good())
            {
                cout << "anole: can't open file '" << path << "': No such file" << endl;
            }
            Parser(fin, path).gen_statements()->codegen(*code);
            auto rd_path = path + ".rd";
            code->print(rd_path);
            code->serialize(ir_path);
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
