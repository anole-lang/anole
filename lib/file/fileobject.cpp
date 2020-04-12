#include <fstream>
#include "fileobject.hpp"
#include "../../src/context.hpp"
#include "../../src/boolobject.hpp"
#include "../../src/stringobject.hpp"
#include "../../src/integerobject.hpp"

using namespace std;
using namespace ice_language;

void _open()
{
    auto path = theCurrentContext->pop<StringObject>();
    auto mode = theCurrentContext->pop<IntegerObject>();

    theCurrentContext->push(make_shared<FileObject>(path->to_str(), mode->value()));
}

static map<string, function<void(FileObject *)>>
built_in_methods_for_file
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
                = theCurrentContext->pop<StringObject>()->to_str();
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
            obj->file().seekg(theCurrentContext->pop<IntegerObject>()->value());
        }
    },
    {"seekp", [](FileObject *obj)
        {
            obj->file().seekp(theCurrentContext->pop<IntegerObject>()->value());
        }
    }
};

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

SPtr<ObjectPtr> FileObject::load_member(const string &name)
{
    if (built_in_methods_for_file.count(name))
    {
        auto &func = built_in_methods_for_file[name];
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
