#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class FunctionObject : public Object
{
  public:
    FunctionObject(SPtr<Scope> pre_scope,
        SPtr<Code> code, Size base,
        Size parameter_num)
      : Object(ObjectType::Func)
      , scope_(std::make_shared<Scope>(pre_scope))
      , code_(code), base_(base)
      , parameter_num_(parameter_num) {}

    SPtr<Scope> scope() { return scope_; }
    SPtr<Code>  code()  { return code_; }
    Size base()  { return base_; }

    String to_str() override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    Address load_member(const String &name) override;
    void call(Size num) override;

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size base_;
    Size parameter_num_;
};
}
