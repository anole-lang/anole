#include "runtime.hpp"

#include "../objects/objects.hpp"

namespace anole
{
Scope::Scope() noexcept
  : pre_scope_(nullptr)
{
    // ...
}

Scope::Scope(SPtr<Scope> pre_scope) noexcept
  : pre_scope_(std::move(pre_scope))
{
    // ...
}

SPtr<Scope> &Scope::pre()
{
    return pre_scope_;
}

Address Scope::create_symbol(const String &name)
{
    Address addr;

    auto find = symbols_.find(name);
    if (find == symbols_.end())
    {
        addr = std::make_shared<Variable>();
        symbols_[name] = addr;
    }
    else
    {
        addr = find->second;
    }
    addr->set_called_name(name);

    return addr;
}

void Scope::create_symbol(const String &name, Object *obj)
{
    symbols_[name] = std::make_shared<Variable>(obj);
}

void Scope::create_symbol(const String &name, Address value)
{
    symbols_[name] = value;
}

Address Scope::load_symbol(const String &name)
{
    auto ptr = find_symbol(name);
    auto res = ptr ? ptr : load_builtin(name);

    if (res)
    {
        res->set_called_name(name);
        return res;
    }
    else
    {
        return create_symbol(name);
    }
}

const std::map<String, Address> &Scope::symbols() const
{
    return symbols_;
}

Address Scope::load_builtin(const String &name)
{
    if (auto func = BuiltInFunctionObject::load_built_in_function(name))
    {
        return std::make_shared<Variable>(func);
    }
    return nullptr;
}

Address Scope::find_symbol(const String &name)
{
    auto res = symbols_.find(name);
    if (res != symbols_.end())
    {
        return res->second;
    }
    else if (pre_scope_)
    {
        return pre_scope_->find_symbol(name);
    }
    else
    {
        return nullptr;
    }
}
}
