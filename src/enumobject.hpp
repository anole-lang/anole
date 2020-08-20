#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class EnumObject : public Object
{
  public:
    EnumObject(SPtr<Scope> scope)
      : Object(ObjectType::Enum)
      , scope_(std::move(scope))
    {
        // ...
    }

    Address load_member(const String &name) override;

    void collect(std::function<void(Scope *)>) override;

  private:
    SPtr<Scope> scope_;
};
}
