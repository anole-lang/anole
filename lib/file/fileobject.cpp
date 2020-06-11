#include <fstream>
#include "fileobject.hpp"
#include "../../src/context.hpp"
#include "../../src/boolobject.hpp"
#include "../../src/stringobject.hpp"
#include "../../src/integerobject.hpp"

using namespace std;
using namespace anole;

void _open()
{
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
map<string, function<void(FileObject *)>>
builtin_methods
{
    {"good", [](FileObject *obj)
        {
            theCurrentContext->push(obj->file().good() ? theTrue : theFalse);
        }
    },
    {"eof", [](FileObject *obj)
        {
            theCurrentContext->push(obj->file().eof() ? theTrue : theFalse);
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
            theCurrentContext->push(
                make_shared<StringObject>(
                    string(1, obj->file().get())));
        }
    },
    {"readline", [](FileObject *obj)
        {
            string line;
            std::getline(obj->file(), line);
            theCurrentContext->push(
                make_shared<StringObject>(line));
        }
    },
    {"write", [](FileObject *obj)
        {
            const auto &str
                = reinterpret_cast<StringObject *>(theCurrentContext->pop().get())->to_str();
            obj->file().write(str.c_str(), str.size());
        }
    },
    {"tellg", [](FileObject *obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(obj->file().tellg()));
        }
    },
    {"tellp", [](FileObject *obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(obj->file().tellp()));
        }
    },
    {"seekg", [](FileObject *obj)
        {
            obj->file().seekg(reinterpret_cast<IntegerObject *>(theCurrentContext->pop().get())->value());
        }
    },
    {"seekp", [](FileObject *obj)
        {
            obj->file().seekp(reinterpret_cast<IntegerObject *>(theCurrentContext->pop().get())->value());
        }
    }
};
}

FileObject::FileObject(const string &path, int64_t mode)
{
    static ios_base::openmode mapping[6]
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
    if (builtin_methods.count(name))
    {
        auto &func = builtin_methods[name];
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([this, func]
            {
                func(this);
            })
        );
    }
    return Object::load_member(name);
}

fstream &FileObject::file()
{
    return file_;
}
