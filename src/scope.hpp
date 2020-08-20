#pragma once

#include "object.hpp"
#include "variable.hpp"
#include "allocator.hpp"
#include "builtinfuncobject.hpp"

#include <map>

namespace anole
{
class Scope
{
    friend class Collector;

  public:
    Scope() : pre_scope_(nullptr) {}
    Scope(SPtr<Scope> pre_scope)
      : pre_scope_(std::move(pre_scope))
    {
        // ...
    }

    SPtr<Scope> &pre()
    {
        return pre_scope_;
    }

    Address create_symbol(const String &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = Allocator<Variable>::alloc();
        }
        return symbols_[name];
    }

    void create_symbol(const String &name, Address value)
    {
        symbols_[name] = value;
    }

    Address load_symbol(const String &name)
    {
        auto ptr = find_symbol(name);
        auto res = ptr ? ptr : load_builtin(name);
        return res ? res : create_symbol(name);
    }

    Address load_builtin(const String &name)
    {
        if (auto func = BuiltInFunctionObject::load_built_in_function(name))
        {
            return Allocator<Variable>::alloc(func);
        }
        return nullptr;
    }

    const std::map<String, Address> &symbols() const
    {
        return symbols_;
    }

  private:
    Address find_symbol(const String &name)
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

    SPtr<Scope> pre_scope_;
    std::map<String, Address> symbols_;
};
}
