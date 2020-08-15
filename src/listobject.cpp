#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#include <map>
#include <utility>

using namespace std;

namespace anole
{
namespace
{
map<String, function<void(SPtr<ListObject> &)>>
lc_builtin_methods_for_list
{
    {"empty", [](SPtr<ListObject> &obj)
        {
            theCurrentContext->push(obj->objects().empty() ? theTrue : theFalse);
        }
    },
    {"size", [](SPtr<ListObject> &obj)
        {
            theCurrentContext->push(make_shared<IntegerObject>(int64_t(obj->objects().size())));
        }
    },
    {"push", [](SPtr<ListObject> &obj)
        {
            obj->append(theCurrentContext->pop());
            theCurrentContext->push(theNone);
        }
    },
    {"pop", [](SPtr<ListObject> &obj)
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            theCurrentContext->push_address(res);
        }
    },
    {"pop_front", [](SPtr<ListObject> &obj)
        {
            auto res = obj->objects().front();
            obj->objects().pop_front();
            theCurrentContext->push_address(res);
        }
    },
    {"front", [](SPtr<ListObject> &obj)
        {
            theCurrentContext->push_address(obj->objects().front());
        }
    },
    {"back", [](SPtr<ListObject> &obj)
        {
            theCurrentContext->push_address(obj->objects().back());
        }
    },
    {"clear", [](SPtr<ListObject> &obj)
        {
            obj->objects().clear();
            theCurrentContext->push(theNone);
        }
    },

    // used by foreach
    {"__iterator__", [](SPtr<ListObject> &obj)
        {
            theCurrentContext
                ->push(make_shared<ListIteratorObject>(obj))
            ;
        }
    }
};

map<String, function<void(SPtr<ListIteratorObject> &)>>
lc_builtin_methods_for_listiterator
{
    // used by foreach
    {"__has_next__", [](SPtr<ListIteratorObject> &obj)
        {
            theCurrentContext
                ->push(obj->has_next() ? theTrue : theFalse)
            ;
        }
    },
    {"__next__", [](SPtr<ListIteratorObject> &obj)
        {
            theCurrentContext
                ->push_address(obj->next())
            ;
        }
    }
};
}

bool ListObject::to_bool()
{
    return !objects_.empty();
}

String ListObject::to_str()
{
    String res = "[";
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

String ListObject::to_key()
{
    return 'l' + to_str();
}

ObjectPtr ListObject::add(ObjectPtr obj)
{
    if (obj->is<ObjectType::List>())
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

Address ListObject::index(ObjectPtr index)
{
    if (index->is<ObjectType::Integer>())
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

Address ListObject::load_member(const String &name)
{
    auto method = lc_builtin_methods_for_list.find(name);
    if (method != lc_builtin_methods_for_list.end())
    {
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([
                ptr = shared_from_this(),
                &func = method->second](Size) mutable
            {
                func(ptr);
            })
        );
    }
    return Object::load_member(name);
}

list<Address> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(ObjectPtr obj)
{
    objects_.push_back(make_shared<ObjectPtr>(obj));
}

Address ListIteratorObject::load_member(const String &name)
{
    auto method = lc_builtin_methods_for_listiterator.find(name);
    if (method != lc_builtin_methods_for_listiterator.end())
    {
        return make_shared<ObjectPtr>(
            make_shared<BuiltInFunctionObject>([
                ptr = shared_from_this(),
                &func = method->second](Size) mutable
            {
                func(ptr);
            })
        );
    }
    return Object::load_member(name);
}
}
