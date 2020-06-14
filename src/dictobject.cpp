#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "dictobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace anole
{
namespace
{
map<string, function<void(SPtr<DictObject> &)>>
builtin_methods
{
    {"empty", [](SPtr<DictObject> &obj)
        {
            theCurrentContext->push(obj->data().empty() ? theTrue : theFalse);
        }
    },
    {"size", [](SPtr<DictObject> &obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(int64_t(obj->data().size())));
        }
    },
    {"at", [](SPtr<DictObject> &obj)
        {
            theCurrentContext->push(*(obj->index(theCurrentContext->pop())));
        }
    },
    {"insert", [](SPtr<DictObject> &obj)
        {
            auto p1 = theCurrentContext->pop();
            auto p2 = theCurrentContext->pop();
            obj->insert(p1, p2);
            theCurrentContext->push(theNone);
        }
    },
    {"erase", [](SPtr<DictObject> &obj)
        {
            obj->data().erase(theCurrentContext->pop());
            theCurrentContext->push(theNone);
        }
    },
    {"clear", [](SPtr<DictObject> &obj)
        {
            obj->data().clear();
            theCurrentContext->push(theNone);
        }
    }
};
}

bool DictObject::to_bool()
{
    return !data_.empty();
}

string DictObject::to_str()
{
    string res = "{";
    for (auto it = data_.begin();
        it != data_.end(); ++it)
    {
        if (it != data_.begin())
        {
            res += ",";
        }
        res += " " + it->first->to_str() + " => " + (*it->second)->to_str();
    }
    return res + " }";
}

string DictObject::to_key()
{
    return 'd' + to_str();
}

Address DictObject::index(ObjectPtr index)
{
    return data_[index];
}

Address DictObject::load_member(const string &name)
{
    auto method = builtin_methods.find(name);
    if (method != builtin_methods.end())
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

DictObject::DataType &DictObject::data()
{
    return data_;
}

void DictObject::insert(ObjectPtr key, ObjectPtr value)
{
    data_[key] = make_shared<ObjectPtr>(value);
}
}
