#include "path.hpp"
#include "../../../src/context.hpp"

using namespace std;
namespace fs = filesystem;
using namespace anole;

void __current_path()
{
    theCurrentContext
        ->push(
            make_shared<PathObject>(
                fs::current_path()
            )
        );
}

vector<string> _FUNCTIONS {
    "__current_path"s
};

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
