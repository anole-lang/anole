#include "objects.hpp"

#include "../runtime/runtime.hpp"

#include <map>
#include <utility>

namespace anole
{
namespace
{
std::map<String, std::function<void(ListObject *)>>
localBuiltinMethodsForList
{
    {"empty", [](ListObject *obj)
        {
            theCurrContext->push(obj->objects().empty() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"size", [](ListObject *obj)
        {
            theCurrContext->push(Allocator<Object>::alloc<IntegerObject>(int64_t(obj->objects().size())));
        }
    },
    {"push", [](ListObject *obj)
        {
            obj->append(theCurrContext->pop_ptr());
            theCurrContext->push(NoneObject::one());
        }
    },
    {"pop", [](ListObject *obj)
        {
            auto res = obj->objects().back();
            obj->objects().pop_back();
            theCurrContext->push(res);
        }
    },
    {"pop_front", [](ListObject *obj)
        {
            auto res = obj->objects().front();
            obj->objects().pop_front();
            theCurrContext->push(res);
        }
    },
    {"front", [](ListObject *obj)
        {
            theCurrContext->push(obj->objects().front());
        }
    },
    {"back", [](ListObject *obj)
        {
            theCurrContext->push(obj->objects().back());
        }
    },
    {"clear", [](ListObject *obj)
        {
            obj->objects().clear();
            theCurrContext->push(NoneObject::one());
        }
    },

    // used by foreach
    {"__iterator__", [](ListObject *obj)
        {
            theCurrContext
                ->push(Allocator<Object>::alloc<ListIteratorObject>(obj))
            ;
        }
    }
};

std::map<String, std::function<void(ListIteratorObject *)>>
localBuiltinMethodsForListIterator
{
    // used by foreach
    {"__has_next__", [](ListIteratorObject *obj)
        {
            theCurrContext
                ->push(obj->has_next() ? BoolObject::the_true() : BoolObject::the_false())
            ;
        }
    },
    {"__next__", [](ListIteratorObject *obj)
        {
            theCurrContext->push(obj->next());
        }
    }
};
}

ListObject::ListObject() noexcept
  : Object(ObjectType::List)
{
    // ...
}

std::list<Address> &ListObject::objects()
{
    return objects_;
}

void ListObject::append(Object *obj)
{
    objects_.push_back(std::make_shared<Variable>(obj));
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
    auto method = localBuiltinMethodsForList.find(name);
    if (method != localBuiltinMethodsForList.end())
    {
        return std::make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>([this, &func = method->second](Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return Object::load_member(name);
}

void ListObject::collect(std::function<void(Object *)> func)
{
    for (auto &addr : objects_)
    {
        func(addr->ptr());
    }
}

ListIteratorObject::ListIteratorObject(ListObject *bind)
  : Object(ObjectType::ListIterator)
  , bind_(bind), current_(bind->objects().begin())
{
    // ...
}

bool ListIteratorObject::has_next()
{
    return current_ != bind_->objects().end();
}

Address ListIteratorObject::next()
{
    return *current_++;
}

Address ListIteratorObject::load_member(const String &name)
{
    auto method = localBuiltinMethodsForListIterator.find(name);
    if (method != localBuiltinMethodsForListIterator.end())
    {
        return std::make_shared<Variable>(
            Allocator<Object>::alloc<BuiltInFunctionObject>(
                [this, &func = method->second]
                (Size) mutable
                {
                    func(this);
                },
                this
            )
        );
    }
    return Object::load_member(name);
}

void ListIteratorObject::collect(std::function<void(Object *)> func)
{
    func(bind_);
}
}
