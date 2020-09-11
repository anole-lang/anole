#include "../path/path.hpp"

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

    auto path_obj = Context::current()->pop_ptr();
    fs::path path;
    if (auto ptr = dynamic_cast<PathObject *>(path_obj))
    {
        path = ptr->path();
    }
    else
    {
        path = path_obj->to_str();
    }

    if (path.is_relative())
    {
        path = Context::current()->current_path() / path;
    }

    auto paths = Allocator<Object>::alloc<ListObject>();
    for (auto &p : fs::directory_iterator(path))
    {
        paths->append(Allocator<Object>::alloc<PathObject>(p.path().lexically_normal()));
    }

    Context::current()->push(paths);
}
}
