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
        SPtr<Code> code, std::size_t base)
      : Object(ObjectType::Func)
      , scope_(std::make_shared<Scope>(pre_scope))
      , code_(code), base_(base){}

    SPtr<Scope> scope() { return scope_; }
    SPtr<Code>  code()  { return code_; }
    std::size_t base()  { return base_; }

    std::string to_str() override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    Address load_member(const std::string &name) override;
    void call(size_t) override;
    void call_tail(size_t) override;

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    std::size_t base_;
};
}
