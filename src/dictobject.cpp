#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "dictobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace anole
{
namespace
{
map<String, function<void(DictObject *)>>
lc_builtin_methods
{
    {"empty", [](DictObject *obj)
        {
            Context::current()->push(obj->data().empty() ? BoolObject::the_true() : BoolObject::the_false());
        }
    },
    {"size", [](DictObject *obj)
        {
            Context::current()->push(make_shared<IntegerObject>(int64_t(obj->data().size())));
        }
    },
    {"at", [](DictObject *obj)
        {
            Context::current()->push(obj->index(Context::current()->pop_sptr())->sptr());
        }
    },
    {"insert", [](DictObject *obj)
        {
            auto p1 = Context::current()->pop_sptr();
            auto p2 = Context::current()->pop_sptr();
            obj->insert(p1, p2);
            Context::current()->push(NoneObject::one());
        }
    },
    {"erase", [](DictObject *obj)
        {
            obj->data().erase(Context::current()->pop_sptr());
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
        res += " " + it->first->to_str() + " => " + it->second->rptr()->to_str();
    }
    return res + " }";
}

String DictObject::to_key()
{
    return 'd' + to_str();
}

Address DictObject::index(ObjectSPtr index)
{
    auto it = data_.find(index);
    if (it != data_.end())
    {
        return it->second;
    }
    /**
     * dict will create an empty target if the key is not recorded
    */
    return data_[index] = Allocator<Variable>::alloc();
}

Address DictObject::load_member(const String &name)
{
    auto method = lc_builtin_methods.find(name);
    if (method != lc_builtin_methods.end())
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
    return index(make_shared<StringObject>(name));
}

void DictObject::collect(function<void(Variable *)> func)
{
    for (auto &key_addr : data_)
    {
        func(key_addr.second);
    }
}

DictObject::DataType &DictObject::data()
{
    return data_;
}

void DictObject::insert(ObjectSPtr key, ObjectSPtr value)
{
    data_[key] = Allocator<Variable>::alloc(value);
}
}
