#ifndef __ANOLE_OBJECTS_THUNK_HPP__
#define __ANOLE_OBJECTS_THUNK_HPP__

#include "object.hpp"

namespace anole
{
class Code;

class ThunkObject : public Object
{
  public:
    ThunkObject(SPtr<Scope> pre_scope,
        SPtr<Code> code, Size base
    );

    void collect(std::function<void(Scope *)> func) override;
    void collect(std::function<void(Object *)> func) override;

    void set_result(Address res);
    bool computed() { return computed_; }
    Address result() { return result_; }
    SPtr<Scope> scope() { return scope_; }
    SPtr<Code> code() { return code_; }
    Size base() { return base_; }

  private:
    bool computed_;
    Address result_;
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size base_;
};
}

#endif
