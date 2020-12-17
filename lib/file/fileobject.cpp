#include "fileobject.hpp"

#include <fstream>

extern "C"
{
void __open(anole::Size n)
{
    if (n != 2)
    {
        throw anole::RuntimeError("function open need 2 arguments");
    }

    auto path = anole::Context::current()->pop_ptr();
    auto mode = anole::Context::current()->pop_ptr();

    anole::Context::current()
        ->push(
            anole::Allocator<anole::Object>::alloc<FileObject>(
                path->to_str(),
                dynamic_cast<anole::IntegerObject *>(mode)->value()
            )
        )
    ;
}
}

namespace
{
std::map<anole::String, std::function<void(FileObject *)>>
lc_builtin_methods
{
    {"good", [](FileObject *obj)
        {
            anole::Context::current()->push(obj->file().good() ? anole::BoolObject::the_true() : anole::BoolObject::the_false());
        }
    },
    {"eof", [](FileObject *obj)
        {
            anole::Context::current()->push(obj->file().eof() ? anole::BoolObject::the_true() : anole::BoolObject::the_false());
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
            anole::Context::current()->push(
                anole::Allocator<anole::Object>::alloc<anole::StringObject>(
                    anole::String(1, obj->file().get())
                )
            );
        }
    },
    {"readline", [](FileObject *obj)
        {
            anole::String line;
            std::getline(obj->file(), line);
            anole::Context::current()->push(anole::Allocator<anole::Object>::
                alloc<anole::StringObject>(line)
            );
        }
    },
    {"write", [](FileObject *obj)
        {
            const auto &str
                = dynamic_cast<anole::StringObject *>(anole::Context::current()->pop_ptr())->to_str()
            ;
            obj->file().write(str.c_str(), str.size());
        }
    },
    {"tellg", [](FileObject *obj)
        {
            anole::Context::current()->push(anole::Allocator<anole::Object>::
                alloc<anole::IntegerObject>(
                    obj->file().tellg()
                )
            );
        }
    },
    {"tellp", [](FileObject *obj)
        {
            anole::Context::current()->push(anole::Allocator<anole::Object>::
                alloc<anole::IntegerObject>(
                    obj->file().tellp()
                )
            );
        }
    },
    {"seekg", [](FileObject *obj)
        {
            obj->file().seekg(dynamic_cast<anole::IntegerObject *>(anole::Context::current()->pop_ptr())->value());
        }
    },
    {"seekp", [](FileObject *obj)
        {
            obj->file().seekp(dynamic_cast<anole::IntegerObject *>(anole::Context::current()->pop_ptr())->value());
        }
    }
};
}

FileObject::FileObject(const anole::String &path, int64_t mode)
  : anole::Object(anole::Object::add_object_type("file"))
{
    static constexpr std::ios_base::openmode mapping[6]
    {
        std::ios_base::app,
        std::ios_base::binary,
        std::ios_base::in,
        std::ios_base::out,
        std::ios_base::trunc,
        std::ios_base::ate
    };

    std::ios_base::openmode mod;
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

anole::Address FileObject::load_member(const anole::String &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
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
    return Object::load_member(name);
}

std::fstream &FileObject::file()
{
    return file_;
}
