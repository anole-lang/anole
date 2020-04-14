#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "dictobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace anole
{
static map<string, function<void(DictObject *)>>
built_in_methods_for_dict
{
    {"empty", [](DictObject *obj)
        {
            theCurrentContext->push(obj->data().empty() ? theTrue : theFalse);
        }
    },
    {"size", [](DictObject *obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(static_cast<int64_t>(obj->data().size())));
        }
    },
    {"at", [](DictObject *obj)
        {
            theCurrentContext->push(*(obj->index(theCurrentContext->pop())));
        }
    },
    {"insert", [](DictObject *obj)
        {
            auto p1 = theCurrentContext->pop();
            auto p2 = theCurrentContext->pop();
            obj->insert(p1, p2);
            theCurrentContext->push(theNone);
        }
    },
    {"erase", [](DictObject *obj)
        {
            obj->data().erase(theCurrentContext->pop());
            theCurrentContext->push(theNone);
        }
    },
    {"clear", [](DictObject *obj)
        {
            obj->data().clear();
            theCurrentContext->push(theNone);
        }
    }
};

DictObject::DictObject() = default;

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

SPtr<ObjectPtr> DictObject::index(ObjectPtr index)
{
    return data_[index];
}

SPtr<ObjectPtr> DictObject::load_member(const string &name)
{
    if (built_in_methods_for_dict.count(name))
    {
        auto &func = built_in_methods_for_dict[name];
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([this, func]()
            {
                func(this);
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
