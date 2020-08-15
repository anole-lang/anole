#pragma once

#include "object.hpp"
#include "builtinfuncobject.hpp"

#include <map>

namespace anole
{
class Scope
{
  public:
    Scope() : pre_scope_(nullptr) {}
    Scope(SPtr<Scope> pre_scope)
      : pre_scope_(pre_scope)
    {
        // ...
    }

    SPtr<Scope> &pre()
    {
        return pre_scope_;
    }

    Address &create_symbol(const String &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = std::make_shared<ObjectPtr>(nullptr);
        }
        return symbols_[name];
    }

    void create_symbol(const String &name, Address value)
    {
        symbols_[name] = std::move(value);
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
            return std::make_shared<ObjectPtr>(func);
        }
        return nullptr;
    }

    const std::map<String, Address> &symbols()
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
