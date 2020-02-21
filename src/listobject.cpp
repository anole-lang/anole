#include <map>
#include <utility>
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
static map<string, pair<size_t, function<ObjectPtr(ListObject *, vector<Ptr<ObjectPtr>>&)>>>
built_in_methods_for_list
{
    {"empty", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            return obj->objects().empty() ? theTrue : theFalse;
        }}
    },
    {"size", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            return make_shared<IntegerObject>(static_cast<int64_t>(obj->objects().size()));
        }}
    },
    {"push", {1,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            obj->append(*objs[0]);
            return nullptr;
        }}
    },
    {"pop", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            return *res;
        }}
    },
    {"front", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            return *obj->objects().front();
        }}
    },
    {"back", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            return *obj->objects().back();
        }}
    },
    {"clear", {0,
        [](ListObject *obj, vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
        {
            obj->objects().clear();
            return nullptr;
        }}
    }
};

ListObject::ListObject() = default;

bool ListObject::to_bool()
{
    return !objects_.empty();
}

string ListObject::to_str()
{
    string res = "[";
    for (size_t i = 0; i < objects_.size(); ++i)
    {
        if (i) res += ", ";
        res += (*objects_[i])->to_str();
    }
    return res + "]";
}

string ListObject::to_key()
{
    return 'l' + to_str();
}

Ptr<ObjectPtr> ListObject::index(ObjectPtr index)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(index))
    {
        return objects_[p->value()];
    }
    else
    {
        throw runtime_error("index should be integer");
    }
}

Ptr<ObjectPtr> ListObject::load_member(const string &name)
{
    if (built_in_methods_for_list.count(name))
    {
        auto &num_func = built_in_methods_for_list[name];
        auto func = num_func.second;
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>(num_func.first,
                [this, func](vector<Ptr<ObjectPtr>> &objs) -> ObjectPtr
                {
                    return func(this, objs);
                }
            )
        );
    }
    return Object::load_member(name);
}

vector<Ptr<ObjectPtr>> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(ObjectPtr obj)
{
    objects_.push_back(make_shared<ObjectPtr>(obj));
}

/*
load_member
{
    if auto builtinmethod =  builtin.load(name)
    {
        return BuiltInFunction([this](vector &objs)
        {
            return builtinmethod(this, objs);
        });
    }
    else
    {
        auto obj = scope_->load(name);
        if (obj = func)
        {
            return method(shared_from_this(), func);
        }
        else return obj;
    }
}
*/
}
