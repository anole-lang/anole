#include <map>
#include <utility>
#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace anole
{
static map<string, function<void(ListObject *)>>
built_in_methods_for_list
{
    {"empty", [](ListObject *obj)
        {
            theCurrentContext->push(obj->objects().empty() ? theTrue : theFalse);
        }
    },
    {"size", [](ListObject *obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(static_cast<int64_t>(obj->objects().size())));
        }
    },
    {"push", [](ListObject *obj)
        {
            obj->append(theCurrentContext->pop());
            theCurrentContext->push(theNone);
        }
    },
    {"pop", [](ListObject *obj)
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            theCurrentContext->push(*res);
        }
    },
    {"pop_front", [](ListObject *obj)
        {
            auto res = obj->objects().front();
            obj->objects().pop_front();
            theCurrentContext->push(*res);
        }
    },
    {"front", [](ListObject *obj)
        {
            theCurrentContext->push(*obj->objects().front());
        }
    },
    {"back", [](ListObject *obj)
        {
            theCurrentContext->push(*obj->objects().back());
        }
    },
    {"clear", [](ListObject *obj)
        {
            obj->objects().clear();
            theCurrentContext->push(theNone);
        }
    }
};

bool ListObject::to_bool()
{
    return !objects_.empty();
}

string ListObject::to_str()
{
    string res = "[";
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
    {
        if (it != objects_.begin())
        {
            res += ", ";
        }
        res += (**it)->to_str();
    }
    return res + "]";
}

string ListObject::to_key()
{
    return 'l' + to_str();
}

ObjectPtr ListObject::add(ObjectPtr obj)
{
    if (obj->type() == ObjectType::List)
    {
        auto p = reinterpret_pointer_cast<ListObject>(obj);
        auto res = make_shared<ListObject>();
        for (auto &obj : objects_)
        {
            res->objects().push_back(obj);
        }
        for (auto &obj : p->objects())
        {
            res->objects().push_back(obj);
        }
        return res;
    }
    else
    {
        throw RuntimeError("expected");
    }
}

SPtr<ObjectPtr> ListObject::index(ObjectPtr index)
{
    if (index->type() == ObjectType::Integer)
    {
        auto it = objects_.begin();
        auto v = reinterpret_pointer_cast<IntegerObject>(index)->value();
        while (v--)
        {
            ++it;
        }
        return *it;
    }
    else
    {
        throw RuntimeError("index should be integer");
    }
}

SPtr<ObjectPtr> ListObject::load_member(const string &name)
{
    if (built_in_methods_for_list.count(name))
    {
        auto &func = built_in_methods_for_list[name];
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([this, func]
            {
                func(this);
            })
        );
    }
    return Object::load_member(name);
}

list<SPtr<ObjectPtr>> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(ObjectPtr obj)
{
    objects_.push_back(make_shared<ObjectPtr>(obj));
}
}
