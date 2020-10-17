#pragma once

#include "object.hpp"

namespace anole
{
class Scope;

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

    SPtr<Scope> &scope() { return scope_; }

  private:
    SPtr<Scope> scope_;
};
}
