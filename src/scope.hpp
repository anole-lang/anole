#pragma once

#include <map>
#include <string>
#include "helper.hpp"
#include "object.hpp"
#include "builtinfuncobject.hpp"

namespace ice_language
{
class Scope
{
  public:
    Scope() : pre_scope_(nullptr) {}
    Scope(SPtr<Scope> pre_scope)
      : pre_scope_(pre_scope) {}

    SPtr<Scope> pre()
    {
        return pre_scope_;
    }

    SPtr<ObjectPtr> create_symbol(const std::string &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = std::make_shared<ObjectPtr>(nullptr);
        }
        return symbols_[name];
    }

    void create_symbol(const std::string &name, SPtr<ObjectPtr> value)
    {
        symbols_[name] = value;
    }

    SPtr<ObjectPtr> load_symbol(const std::string &name)
    {
        auto ptr = find_symbol(name);
        auto res = ptr ? ptr : load_builtin(name);
        return res ? res : create_symbol(name);
    }

    SPtr<ObjectPtr> load_builtin(const std::string &name)
    {
        if (auto func = BuiltInFunctionObject::load_built_in_function(name))
        {
            return std::make_shared<ObjectPtr>(func);
        }
        return nullptr;
    }

    const std::map<std::string, SPtr<ObjectPtr>> &symbols()
    {
        return symbols_;
    }

  private:
    SPtr<ObjectPtr> find_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
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
    std::map<std::string, SPtr<ObjectPtr>> symbols_;
};
}
