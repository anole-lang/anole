#pragma once

#include "object.hpp"

namespace anole
{
class Scope;

class ClassObject : public Object
{
  public:
    ClassObject(String name,
        SPtr<Scope> pre_scope
    );

    void call(Size num) override;

    // collect

    SPtr<Scope> &scope() { return scope_; }

  private:
    String name_;
    SPtr<Scope> scope_;
};
} // namespace anole
