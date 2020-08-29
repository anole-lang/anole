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

    Context::current()->push(
        make_shared<PathObject>(
            fs::current_path())
    );
}

void __is_directory(Size n)
{
    if (n != 1)
    {
        throw RuntimeError("function current_path need 1 argument");
    }

    auto path_obj = Context::current()->pop_rptr();
    fs::path path;
    if (path_obj->is(Object::add_object_type("path")))
    {
        path = static_cast<PathObject *>(path_obj)->path();
    }
    else
    {
        path = path_obj->to_str();
    }

    if (path.is_relative())
    {
        path = Context::current()->current_path() / path;
    }

    Context::current()->push(fs::is_directory(path) ? BoolObject::the_true() : BoolObject::the_false());
}
}

namespace
{
map<String, function<void(PathObject *)>>
lc_builtin_methods
{
    {"is_directory", [](PathObject *obj)
        {
            Context::current()->push(fs::is_directory(obj->path()) ? BoolObject::the_true() : BoolObject::the_false());
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
        return Allocator<Variable>::alloc(
            make_shared<BuiltInFunctionObject>([
                    sptr = shared_from_this(),
                    &func = method->second](Size) mutable
                {
                    func(sptr.get());
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
