#include "../path/path.hpp"
#include "../../../src/context.hpp"
#include "../../../src/listobject.hpp"
#include "../../../src/stringobject.hpp"

#include <string>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = filesystem;
using namespace anole;

extern "C"
{
vector<String> _FUNCTIONS
{
    "__read_dir"s
};

void __read_dir(Size n)
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
        paths->append(make_shared<PathObject>(p.path().lexically_normal()));
    }

    theCurrentContext->push(paths);
}
}
