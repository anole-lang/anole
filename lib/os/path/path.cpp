#include "path.hpp"

namespace fs = std::filesystem;

extern "C"
{
std::vector<anole::String> _FUNCTIONS
{
    "__current_path",
    "__is_directory"
};

void __current_path(anole::Size n)
{
    if (n != 0)
    {
        throw anole::RuntimeError("function current_path need no arguments");
    }

    anole::theCurrContext->push(
        anole::Allocator<anole::Object>::alloc<PathObject>(
            fs::current_path()
        )
    );
}

void __is_directory(anole::Size n)
{
    if (n != 1)
    {
        throw anole::RuntimeError("function current_path need 1 argument");
    }

    auto path_obj = anole::theCurrContext->pop_ptr();
    fs::path path;
    if (path_obj->is(anole::Object::add_object_type("path")))
    {
        path = static_cast<PathObject *>(path_obj)->path();
    }
    else
    {
        path = path_obj->to_str();
    }

    if (path.is_relative())
    {
        path = anole::theCurrContext->current_path() / path;
    }

    anole::theCurrContext->push(
        fs::is_directory(path)
        ? anole::BoolObject::the_true()
        : anole::BoolObject::the_false()
    );
}
}

namespace
{
std::map<anole::String, std::function<void(PathObject *)>>
localBuiltinMethods
{
    {"is_directory", [](PathObject *obj)
        {
            anole::theCurrContext->push(
                fs::is_directory(obj->path())
                ? anole::BoolObject::the_true()
                : anole::BoolObject::the_false()
            );
        }
    }
};
}

PathObject::PathObject(fs::path path)
  : anole::Object(anole::Object::add_object_type("path"))
  , path_(std::move(path))
{
    // ...
}

anole::Address PathObject::load_member(const anole::String &name)
{
    auto method = localBuiltinMethods.find(name);
    if (method != localBuiltinMethods.end())
    {
        return std::make_shared<anole::Variable>(
            anole::Allocator<anole::Object>::alloc<anole::BuiltInFunctionObject>(
                [this, &func = method->second]
                (anole::Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return anole::Object::load_member(name);
}

anole::String PathObject::to_str()
{
    return path_.string();
}

fs::path &PathObject::path()
{
    return path_;
}
