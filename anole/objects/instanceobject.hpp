#ifndef __ANOLE_OBJECTS_INSTANCE_HPP__
#define __ANOLE_OBJECTS_INSTANCE_HPP__

#include "object.hpp"

namespace anole
{
class InstanceObject : public Object
{
  public:
    InstanceObject();

    SPtr<Scope> &scope();

  public:
    Address load_member(const String &name) override;

    void collect(std::function<void(Scope *)>) override;

  private:
    SPtr<Scope> scope_;
};
} // namespace anole

#endif
