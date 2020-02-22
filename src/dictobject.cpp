#include "noneobject.hpp"
#include "boolobject.hpp"
#include "dictobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
static map<string, pair<size_t, function<ObjectPtr(DictObject *, vector<ObjectPtr>&)>>>
built_in_methods_for_dict
{
    {"empty", {0,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            return obj->data().empty() ? theTrue : theFalse;
        }}
    },
    {"size", {0,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            return make_shared<IntegerObject>(static_cast<int64_t>(obj->data().size()));
        }}
    },
    {"at", {1,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            return *(obj->index(objs[0]));
        }}
    },
    {"insert", {2,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            obj->insert(objs[0], objs[1]);
            return theNone;
        }}
    },
    {"erase", {1,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            obj->data().erase(objs[0]);
            return theNone;
        }}
    },
    {"clear", {0,
        [](DictObject *obj, vector<ObjectPtr> &objs) -> ObjectPtr
        {
            obj->data().clear();
            return theNone;
        }}
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

Ptr<ObjectPtr> DictObject::index(ObjectPtr index)
{
    return data_[index];
}

Ptr<ObjectPtr> DictObject::load_member(const string &name)
{
    if (built_in_methods_for_dict.count(name))
    {
        auto &num_func = built_in_methods_for_dict[name];
        auto func = num_func.second;
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>(num_func.first,
                [this, func](vector<ObjectPtr> &objs) -> ObjectPtr
                {
                    return func(this, objs);
                }
            )
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
