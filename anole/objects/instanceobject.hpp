#ifndef __ANOLE_OBJECTS_INSTANCE_HPP__
#define __ANOLE_OBJECTS_INSTANCE_HPP__

#include "object.hpp"

namespace anole
{
class InstanceObject : public Object
{
  public:
    InstanceObject();

    Address load_member(const String &name) override;

    SPtr<Scope> &scope() { return scope_; }

  private:
    SPtr<Scope> scope_;
};
} // namespace anole

#endif
