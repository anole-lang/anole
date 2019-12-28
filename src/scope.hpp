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

    Ptr<ObjectPtr> create_symbol(std::size_t id)
    {
        if (!symbols_.count(id))
        {
            symbols_[id] = std::make_shared<ObjectPtr>(nullptr);
        }
        return symbols_[id];
    }

    Ptr<ObjectPtr> load_symbol(std::size_t id)
    {
        auto ptr = find_symbol(id);
        return ptr ? ptr : nullptr;
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
    Ptr<ObjectPtr> find_symbol(std::size_t id)
    {
        if (symbols_.count(id))
        {
            return symbols_[id];
        }
        else if (pre_scope_)
        {
            return pre_scope_->find_symbol(id);
        }
        else
        {
            return nullptr;
        }
    }

    Ptr<Scope> pre_scope_;
    std::map<std::size_t, Ptr<ObjectPtr>> symbols_;
};
}
