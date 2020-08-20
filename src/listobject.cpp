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
            Context::current()->push(obj->objects().empty() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"size", [](SPtr<ListObject> &obj)
        {
            Context::current()->push(make_shared<IntegerObject>(int64_t(obj->objects().size())));
        }
    },
    {"push", [](SPtr<ListObject> &obj)
        {
            obj->append(Context::current()->pop());
            Context::current()->push(NoneObject::one());
        }
    },
    {"pop", [](SPtr<ListObject> &obj)
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            Context::current()->push_address(res);
        }
    },
    {"pop_front", [](SPtr<ListObject> &obj)
        {
            auto res = obj->objects().front();
            obj->objects().pop_front();
            Context::current()->push_address(res);
        }
    },
    {"front", [](SPtr<ListObject> &obj)
        {
            Context::current()->push_address(obj->objects().front());
        }
    },
    {"back", [](SPtr<ListObject> &obj)
        {
            Context::current()->push_address(obj->objects().back());
        }
    },
    {"clear", [](SPtr<ListObject> &obj)
        {
            obj->objects().clear();
            Context::current()->push(NoneObject::one());
        }
    },

    // used by foreach
    {"__iterator__", [](SPtr<ListObject> &obj)
        {
            Context::current()
                ->push(make_shared<ListIteratorObject>(obj))
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
            Context::current()
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
        res += (*it)->rptr()->to_str();
    }
    return res + "]";
}

String ListObject::to_key()
{
    return 'l' + to_str();
}

ObjectSPtr ListObject::add(ObjectRawPtr obj)
{
    if (obj->is<ObjectType::List>())
    {
        auto p = reinterpret_cast<ListObject *>(obj);
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

Address ListObject::index(ObjectSPtr index)
{
    if (index->is<ObjectType::Integer>())
    {
        auto it = objects_.begin();
        auto v = reinterpret_cast<IntegerObject *>(index.get())->value();
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
        return Allocator<Variable>::alloc(
            make_shared<BuiltInFunctionObject>([
                    sptr = shared_from_this(),
                    &func = method->second](Size) mutable
                {
                    func(sptr);
                })
        );
    }
    return Object::load_member(name);
}

void ListObject::collect(function<void(Variable *)> func)
{
    for (auto &addr : objects_)
    {
        func(addr);
    }
}

list<Address> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(ObjectSPtr obj)
{
    objects_.push_back(Allocator<Variable>::alloc(obj));
}

Address ListIteratorObject::load_member(const String &name)
{
    auto method = lc_builtin_methods_for_listiterator.find(name);
    if (method != lc_builtin_methods_for_listiterator.end())
    {
        return Allocator<Variable>::alloc(
            make_shared<BuiltInFunctionObject>([
                    sptr = shared_from_this(),
                    &func = method->second](Size) mutable
                {
                    func(sptr.get());
                })
        );
    }
    return Object::load_member(name);
}

void ListIteratorObject::collect(function<void(Variable *)> func)
{
    for (auto &addr : bind_->objects())
    {
        func(addr);
    }
}
}
