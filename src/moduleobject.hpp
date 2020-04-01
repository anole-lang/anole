#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
class ModuleObject : public Object
{
  public:
    ModuleObject(Ptr<Scope> scope)
      : scope_(scope) {}
    Ptr<ObjectPtr> load_member(const std::string &name) override;

  private:
    Ptr<Scope> scope_;
};
}
