#include "fileobject.hpp"
#include "../../src/context.hpp"
#include "../../src/boolobject.hpp"
#include "../../src/stringobject.hpp"
#include "../../src/integerobject.hpp"

#include <fstream>

using namespace std;
using namespace anole;

extern "C"
{
void __open(Size n)
{
    if (n != 2)
    {
        throw RuntimeError("function open need 2 arguments");
    }

    auto path = Context::current()->pop();
    auto mode = Context::current()->pop();

    Context::current()
        ->push(
            Allocator<Object>::alloc<FileObject>(
                path->to_str(),
                reinterpret_cast<IntegerObject *>(mode)->value()))
    ;
}
}

namespace
{
map<String, function<void(FileObject *)>>
lc_builtin_methods
{
    {"good", [](FileObject *obj)
        {
            Context::current()->push(obj->file().good() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"eof", [](FileObject *obj)
        {
            Context::current()->push(obj->file().eof() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"close", [](FileObject *obj)
        {
            obj->file().close();
        }
    },
    {"flush", [](FileObject *obj)
        {
            obj->file().flush();
        }
    },
    {"read", [](FileObject *obj)
        {
            Context::current()->push(
                Allocator<Object>::alloc<StringObject>(
                    String(1, obj->file().get()))
            );
        }
    },
    {"readline", [](FileObject *obj)
        {
            String line;
            std::getline(obj->file(), line);
            Context::current()->push(
                Allocator<Object>::alloc<StringObject>(line)
            );
        }
    },
    {"write", [](FileObject *obj)
        {
            const auto &str
                = reinterpret_cast<StringObject *>(Context::current()->pop())->to_str()
            ;
            obj->file().write(str.c_str(), str.size());
        }
    },
    {"tellg", [](FileObject *obj)
        {
            Context::current()->push(Allocator<Object>::alloc<IntegerObject>(obj->file().tellg()));
        }
    },
    {"tellp", [](FileObject *obj)
        {
            Context::current()->push(Allocator<Object>::alloc<IntegerObject>(obj->file().tellp()));
        }
    },
    {"seekg", [](FileObject *obj)
        {
            obj->file().seekg(reinterpret_cast<IntegerObject *>(Context::current()->pop())->value());
        }
    },
    {"seekp", [](FileObject *obj)
        {
            obj->file().seekp(reinterpret_cast<IntegerObject *>(Context::current()->pop())->value());
        }
    }
};
}

FileObject::FileObject(const String &path, int64_t mode)
  : Object(Object::add_object_type("file"))
{
    static const ios_base::openmode mapping[6]
    {
        ios_base::app,
        ios_base::binary,
        ios_base::in,
        ios_base::out,
        ios_base::trunc,
        ios_base::ate
    };

    ios_base::openmode mod;
    bool assigned = false;
    for (int i = 0; i < 6; ++i)
    {
        if ((mode & (1 << i)))
        {
            if (assigned)
            {
                mod |= mapping[i];
            }
            else
            {
                mod = mapping[i];
                assigned = true;
            }
        }
    }

    file_.open(path, mod);
}

Address FileObject::load_member(const String &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
    {
        return Allocator<Variable>::alloc(
            Allocator<Object>::alloc<BuiltInFunctionObject>([this,
                    &func = method->second](Size) mutable
                {
                    func(this);
                },
                this)
        );
    }
    return Object::load_member(name);
}

fstream &FileObject::file()
{
    return file_;
}
