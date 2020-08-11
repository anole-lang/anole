#include "path.hpp"
#include "../../../src/context.hpp"
#include "../../../src/boolobject.hpp"

using namespace std;
namespace fs = filesystem;
using namespace anole;

extern "C"
{
vector<String> _FUNCTIONS
{
    "__current_path"s,
    "__is_directory"s
};

void __current_path(Size n)
{
    if (n != 0)
    {
        throw RuntimeError("function current_path need no arguments");
    }

    theCurrentContext
        ->push(
            make_shared<PathObject>(
                fs::current_path()));
}

void __is_directory(Size n)
{
    if (n != 1)
    {
        throw RuntimeError("function current_path need 1 argument");
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

    theCurrentContext->push(fs::is_directory(path) ? theTrue : theFalse);
}
}

namespace
{
map<String, function<void(SPtr<PathObject> &)>>
lc_builtin_methods
{
    {"is_directory", [](SPtr<PathObject> &obj)
        {
            theCurrentContext->push(fs::is_directory(obj->path()) ? theTrue : theFalse);
        }
    }
};
}

PathObject::PathObject(fs::path path)
  : Object(Object::add_object_type("path"))
  , path_(move(path)) {}

Address PathObject::load_member(const String &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
    {
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([
                ptr = shared_from_this(),
                &func = method->second](Size) mutable
            {
                func(ptr);
            })
        );
    }
    return Object::load_member(name);
}

String PathObject::to_str()
{
    return path_.string();
}

fs::path &PathObject::path()
{
    return path_;
}
