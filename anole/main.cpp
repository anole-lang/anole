#include "repl.hpp"
#include "anole.hpp"
#include "argparse.hpp"

#include <fstream>
#include <algorithm>

using namespace std;
using namespace anole;

namespace fs = std::filesystem;

namespace
{
bool ends_with(const string_view &str,
    const string_view &target)
{
    if (str.size() < target.size())
    {
        return false;
    }

    for (Size i = 0, j = str.size() - target.size();
        i < target.size(); ++i, ++j)
    {
        if (target[i] != str[j])
        {
            return false;
        }
    }

    return true;
}
}

int main(int argc, char *argv[]) try
{
    if (argc == 1)
    {
        replrun::run();
    }
    else
    {
        /**
         * find where the first anole file is
         *  anole [-r] (file) [arg1[ arg2[ ...]]]
         *
         * just find it by the extension ".anole"
        */
        int file_pos;
        for (file_pos = 0; file_pos < argc; ++file_pos)
        {
            if (ends_with(argv[file_pos], ".anole"sv))
            {
                break;
            }
        }

        ArgumentParser parser("anole");
        parser.add_argument("file");
        parser.add_argument("-r")
              .default_value(false)
              .implict_value(true)
        ;
        parser.add_argument("--version")
              .default_value(false)
              .implict_value(true)
        ;
        parser.parse(min(file_pos + 1, argc), argv);

        if (parser.get<bool>("version"))
        {
            printf("Anole %s\n", Version::literal);
            return 0;
        }

        Context::set_args(argc, argv, file_pos);

        auto path = fs::path(parser.get("file"));
        if (path.extension().string() != ".anole")
        {
            path /= "__init__.anole";
        }
        /**
         * TODO:
         *  distinguish different paths:
         *   current path for context to share,
         *   the path of code binds to the source code,
         *   use the latter only when using other modules
        */
        theCurrContext = make_shared<Context>(make_shared<Code>(path.string()),
            path.parent_path().relative_path()
        );

        try
        {
            AnoleModuleObject mod{path};
            if (!mod.good())
            {
                cout << "anole: cannot open file " << path << endl;
            }
            else if (parser.get<bool>("r"))
            {
                auto rd_path = path;
                rd_path += ".rd";
                mod.code()->print(rd_path);
            }
        }
        catch (const exception& e)
        {
            cerr << e.what() << endl;
        }
    }
    return 0;
}
catch (...)
{
    cout << "invalid command-line argument(s)" << endl;
}
