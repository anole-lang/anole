#pragma once

#include <map>
#include <string>
#include "helper.hpp"

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

    void create_symbol(const std::string &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = std::make_shared<VoidPtr>(nullptr);
        }
    }

    Ptr<VoidPtr> load_symbol(const std::string &name)
    {
        auto ptr = find_symbol(name);
        return ptr ? ptr
             : (symbols_[name] = std::make_shared<VoidPtr>(nullptr));
    }

  private:
    Ptr<VoidPtr> find_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
        }
        else
        {
            return pre_scope_
                 ? pre_scope_->find_symbol(name)
                 : nullptr;
        }
    }

    Ptr<Scope> pre_scope_;
    std::map<std::string, Ptr<VoidPtr>> symbols_;
};
}
