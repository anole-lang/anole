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
    Scope(Ptr<Scope> pre_scope)
      : pre_scope_(pre_scope) {}

    Ptr<Scope> pre()
    {
        return pre_scope_;
    }

    Ptr<ObjectPtr> create_symbol(const std::string &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = std::make_shared<ObjectPtr>(nullptr);
        }
        return symbols_[name];
    }

    Ptr<ObjectPtr> load_symbol(const std::string &name)
    {
        auto ptr = find_symbol(name);
        return ptr ? ptr : load_builtin(name);
    }

    Ptr<ObjectPtr> load_builtin(const std::string &name)
    {
        if (auto func = BuiltInFunctionObject::load_built_in_function(name))
        {
            return std::make_shared<ObjectPtr>(func);
        }
        return nullptr;
    }

  private:
    Ptr<ObjectPtr> find_symbol(const std::string &name)
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

    Ptr<Scope> pre_scope_;
    std::map<std::string, Ptr<ObjectPtr>> symbols_;
};
}
