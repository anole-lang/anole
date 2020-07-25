#include "path.hpp"
#include "../../../src/context.hpp"

using namespace std;
namespace fs = filesystem;
using namespace anole;

extern "C"
{
extern vector<string> _FUNCTIONS;
void __current_path(size_t);
}

vector<string> _FUNCTIONS
{
    "__current_path"s
};

void __current_path(size_t n)
{
    if (n != 0)
    {
        throw RuntimeError("function current_path need no arguments");
    }

    theCurrentContext
        ->push(
            make_shared<PathObject>(
                fs::current_path()
            )
        );
}

namespace
{

}

PathObject::PathObject(fs::path path)
  : path_(move(path)) {}

Address PathObject::load_member(const string &name)
{
    return nullptr;
}

string PathObject::to_str()
{
    return path_.string();
}

fs::path &PathObject::path()
{
    return path_;
}
