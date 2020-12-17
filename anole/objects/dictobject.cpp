#include "objects.hpp"

#include "../runtime/runtime.hpp"

namespace anole
{
namespace
{
std::map<String, std::function<void(DictObject *)>>
lc_builtin_methods
{
    {"empty", [](DictObject *obj)
        {
            Context::current()->push(obj->data().empty() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"size", [](DictObject *obj)
        {
            Context::current()->push(Allocator<Object>::alloc<IntegerObject>(int64_t(obj->data().size())));
        }
    },
    {"at", [](DictObject *obj)
        {
            Context::current()->push(obj->index(Context::current()->pop_ptr())->ptr());
        }
    },
    {"insert", [](DictObject *obj)
        {
            auto p1 = Context::current()->pop_ptr();
            auto p2 = Context::current()->pop_ptr();
            obj->insert(p1, p2);
            Context::current()->push(NoneObject::one());
        }
    },
    {"erase", [](DictObject *obj)
        {
            obj->data().erase(Context::current()->pop_ptr());
            Context::current()->push(NoneObject::one());
        }
    },
    {"clear", [](DictObject *obj)
        {
            obj->data().clear();
            Context::current()->push(NoneObject::one());
        }
    }
};
}

bool DictObject::ObjectCmp::operator()(Object *lhs, Object *rhs) const
{
    return lhs->to_key() < rhs->to_key();
}

DictObject::DictObject() noexcept
  : Object(ObjectType::Dict)
{
    // ...
}

DictObject::DataType &DictObject::data()
{
    return data_;
}

void DictObject::insert(Object *key, Object *value)
{
    data_[key] = std::make_shared<Variable>(value);
}

bool DictObject::to_bool()
{
    return !data_.empty();
}

String DictObject::to_str()
{
    String res = "{";
    for (auto it = data_.begin();
        it != data_.end(); ++it)
    {
        if (it != data_.begin())
        {
            res += ",";
        }
        res += " " + it->first->to_str() + " => " + it->second->ptr()->to_str();
    }
    return res + " }";
}

String DictObject::to_key()
{
    return 'd' + to_str();
}

Address DictObject::index(Object *index)
{
    auto it = data_.find(index);
    if (it != data_.end())
    {
        return it->second;
    }
    /**
     * dict will create an empty target if the key is not recorded
    */
    return data_[index] = std::make_shared<Variable>();
}

Address DictObject::load_member(const String &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
    {
        return std::make_shared<Variable>(
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
    return index(Allocator<Object>::alloc<StringObject>(name));
}

void DictObject::collect(std::function<void(Object *)> func)
{
    for (auto &key_addr : data_)
    {
        func(key_addr.first);
        func(key_addr.second->ptr());
    }
}
}
