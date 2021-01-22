#include "../path/path.hpp"

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

extern "C"
{
std::vector<anole::String> _FUNCTIONS
{
    "__read_dir"
};

void __read_dir(anole::Size n)
{
    if (n != 1)
    {
        throw anole::RuntimeError("function read_dir need only one argument");
    }

    auto path_obj = anole::theCurrContext->pop_ptr();
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
        path = *anole::theWorkingPath / path;
    }

    auto paths = anole::Allocator<anole::Object>::alloc<anole::ListObject>();
    for (auto &p : fs::directory_iterator(path))
    {
        paths->append(anole::Allocator<anole::Object>::
            alloc<PathObject>(
                p.path().lexically_normal()
            )
        );
    }

    anole::theCurrContext->push(paths);
}
}
