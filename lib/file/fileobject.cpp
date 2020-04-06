#include <fstream>
#include "fileobject.hpp"
#include "../../src/context.hpp"
#include "../../src/stringobject.hpp"

using namespace std;
using namespace ice_language;

void _open()
{
    auto path = theCurrentContext->pop<StringObject>();
    auto mode = theCurrentContext->pop<StringObject>();

    theCurrentContext->push(make_shared<FileObject>(path->to_str(), mode->to_str()));
}

static map<string, function<void(FileObject *)>>
built_in_methods_for_file
{
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
    }
};

FileObject::FileObject(const string &path, const string &mode)
{
    if (mode == "r")
    {
        file_.open(path, ios_base::in);
    }
    else
    {
        file_.open(path, ios_base::out);
    }
}

Ptr<ObjectPtr> FileObject::load_member(const string &name)
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
