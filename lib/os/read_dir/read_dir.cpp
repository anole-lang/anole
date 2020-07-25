#include <string>
#include <vector>
#include <filesystem>
#include "../path/path.hpp"
#include "../../../src/context.hpp"
#include "../../../src/listobject.hpp"
#include "../../../src/stringobject.hpp"

using namespace std;
namespace fs = filesystem;
using namespace anole;

extern "C"
{
extern vector<string> _FUNCTIONS;
void __read_dir(size_t);
}

vector<string> _FUNCTIONS
{
    "__read_dir"s
};

void __read_dir(size_t n)
{
    if (n != 1)
    {
        throw RuntimeError("function read_dir need only one argument");
    }

    auto path_obj = theCurrentContext->pop();
    fs::path path;
    if (auto ptr = dynamic_cast<PathObject*>(path_obj.get()))
    {
        path = ptr->path();
    }
    else
    {
        path = path_obj->to_str();
    }

    if (path.is_relative())
    {
        path = theCurrentContext->current_path() / path;
    }

    auto paths = make_shared<ListObject>();
    for (auto &p : fs::directory_iterator(path))
    {
        paths->append(make_shared<PathObject>(p.path()));
    }

    theCurrentContext->push(paths);
}
