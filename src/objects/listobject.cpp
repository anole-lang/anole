#include "objects.hpp"

#include "../runtime/runtime.hpp"

#include <map>
#include <utility>

using namespace std;

namespace anole
{
namespace
{
map<String, function<void(ListObject *)>>
lc_builtin_methods_for_list
{
    {"empty", [](ListObject *obj)
        {
            Context::current()->push(obj->objects().empty() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"size", [](ListObject *obj)
        {
            Context::current()->push(Allocator<Object>::alloc<IntegerObject>(int64_t(obj->objects().size())));
        }
    },
    {"push", [](ListObject *obj)
        {
            obj->append(Context::current()->pop_ptr());
            Context::current()->push(NoneObject::one());
        }
    },
    {"pop", [](ListObject *obj)
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            Context::current()->push(res);
        }
    },
    {"pop_front", [](ListObject *obj)
        {
            auto res = obj->objects().front();
            obj->objects().pop_front();
            Context::current()->push(res);
        }
    },
    {"front", [](ListObject *obj)
        {
            Context::current()->push(obj->objects().front());
        }
    },
    {"back", [](ListObject *obj)
        {
            Context::current()->push(obj->objects().back());
        }
    },
    {"clear", [](ListObject *obj)
        {
            obj->objects().clear();
            Context::current()->push(NoneObject::one());
        }
    },

    // used by foreach
    {"__iterator__", [](ListObject *obj)
        {
            Context::current()
                ->push(Allocator<Object>::alloc<ListIteratorObject>(obj))
            ;
        }
    }
};

map<String, function<void(ListIteratorObject *)>>
lc_builtin_methods_for_listiterator
{
    // used by foreach
    {"__has_next__", [](ListIteratorObject *obj)
        {
            Context::current()
                ->push(obj->has_next() ? BoolObject::the_true() : BoolObject::the_false())
            ;
        }
    },
    {"__next__", [](ListIteratorObject *obj)
        {
            Context::current()->push(obj->next());
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
        res += (*it)->ptr()->to_str();
    }
    return res + "]";
}

String ListObject::to_key()
{
    return 'l' + to_str();
}

Object *ListObject::add(Object *obj)
{
    if (obj->is<ObjectType::List>())
    {
        auto p = reinterpret_cast<ListObject *>(obj);
        auto res = Allocator<Object>::alloc<ListObject>();
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

Address ListObject::index(Object *index)
{
    if (index->is<ObjectType::Integer>())
    {
        auto it = objects_.begin();
        auto v = reinterpret_cast<IntegerObject *>(index)->value();
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
        return make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>([
                    this,
                    &func = method->second](Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return Object::load_member(name);
}

void ListObject::collect(function<void(Object *)> func)
{
    for (auto &addr : objects_)
    {
        func(addr->ptr());
    }
}

list<Address> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(Object *obj)
{
    objects_.push_back(make_shared<Variable>(obj));
}

Address ListIteratorObject::load_member(const String &name)
{
    auto method = lc_builtin_methods_for_listiterator.find(name);
    if (method != lc_builtin_methods_for_listiterator.end())
    {
        return make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>([
                    this,
                    &func = method->second](Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return Object::load_member(name);
}

void ListIteratorObject::collect(function<void(Object *)> func)
{
    func(bind_);
}
}
