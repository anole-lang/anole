#ifndef __ANOLE_OBJECTS_FUNC_HPP__
#define __ANOLE_OBJECTS_FUNC_HPP__

#include "object.hpp"

namespace anole
{
class Code;

class FunctionObject : public Object
{
  public:
    FunctionObject(SPtr<Scope> pre_scope,
        SPtr<Code> code, Size base,
        Size parameter_num
    );

    String to_str() override;
    Address load_member(const String &name) override;
    void call(Size num) override;
    bool is_callable() override;

    void collect(std::function<void(Scope *)>) override;

    SPtr<Scope> scope() { return scope_; }
    SPtr<Code>  code()  { return code_; }
    Size base() { return base_; }

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size base_;
    Size parameter_num_;
};
}

#endif
