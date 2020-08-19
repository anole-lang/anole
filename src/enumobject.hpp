#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class EnumObject : public Object
{
  public:
    EnumObject(Scope *scope)
      : Object(ObjectType::Enum)
      , scope_(scope)
    {
        // ...
    }

    Address load_member(const String &name) override;

  private:
    Scope *scope_;
};
}
