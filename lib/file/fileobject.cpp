#include <fstream>
#include "fileobject.hpp"
#include "../../src/context.hpp"
#include "../../src/boolobject.hpp"
#include "../../src/stringobject.hpp"
#include "../../src/integerobject.hpp"

using namespace std;
using namespace anole;

void __open(size_t n)
{
    if (n != 2)
    {
        throw RuntimeError("function open need 2 arguments");
    }

    auto path = theCurrentContext->pop();
    auto mode = theCurrentContext->pop();

    theCurrentContext
        ->push(
            make_shared<FileObject>(
                path->to_str(),
                reinterpret_cast<IntegerObject *>(mode.get())->value()
            )
        );
}

namespace
{
map<string, function<void(SPtr<FileObject> &)>>
lc_builtin_methods
{
    {"good", [](SPtr<FileObject> &obj)
        {
            theCurrentContext->push(obj->file().good() ? theTrue : theFalse);
        }
    },
    {"eof", [](SPtr<FileObject> &obj)
        {
            theCurrentContext->push(obj->file().eof() ? theTrue : theFalse);
        }
    },
    {"close", [](SPtr<FileObject> &obj)
        {
            obj->file().close();
        }
    },
    {"flush", [](SPtr<FileObject> &obj)
        {
            obj->file().flush();
        }
    },
    {"read", [](SPtr<FileObject> &obj)
        {
            theCurrentContext->push(
                make_shared<StringObject>(
                    string(1, obj->file().get())));
        }
    },
    {"readline", [](SPtr<FileObject> &obj)
        {
            string line;
            std::getline(obj->file(), line);
            theCurrentContext->push(
                make_shared<StringObject>(line));
        }
    },
    {"write", [](SPtr<FileObject> &obj)
        {
            const auto &str
                = reinterpret_cast<StringObject *>(theCurrentContext->pop().get())->to_str();
            obj->file().write(str.c_str(), str.size());
        }
    },
    {"tellg", [](SPtr<FileObject> &obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(obj->file().tellg()));
        }
    },
    {"tellp", [](SPtr<FileObject> &obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(obj->file().tellp()));
        }
    },
    {"seekg", [](SPtr<FileObject> &obj)
        {
            obj->file().seekg(reinterpret_cast<IntegerObject *>(theCurrentContext->pop().get())->value());
        }
    },
    {"seekp", [](SPtr<FileObject> &obj)
        {
            obj->file().seekp(reinterpret_cast<IntegerObject *>(theCurrentContext->pop().get())->value());
        }
    }
};
}

FileObject::FileObject(const string &path, int64_t mode)
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

Address FileObject::load_member(const string &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
    {
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([
                ptr = shared_from_this(),
                &func = method->second](size_t) mutable
            {
                func(ptr);
            })
        );
    }
    return Object::load_member(name);
}

fstream &FileObject::file()
{
    return file_;
}
