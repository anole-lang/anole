#ifndef __ANOLE_OBJECTS_THUNK_HPP__
#define __ANOLE_OBJECTS_THUNK_HPP__

#include "object.hpp"

namespace anole
{
class Code;

class ThunkObject : public Object
{
  public:
    ThunkObject(SPtr<Scope> pre_scope, SPtr<Code> code, Size base);

    void set_result(Address res);
    bool computed() const;
    Address result();
    SPtr<Scope> scope();
    SPtr<Code> code();
    Size base() const;

  public:
    void collect(std::function<void(Scope *)> func) override;
    void collect(std::function<void(Object *)> func) override;

  private:
    bool computed_;
    Address result_;
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size base_;
};
}

#endif
