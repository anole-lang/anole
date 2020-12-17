#ifndef __ANOLE_OBJECTS_ENUM_HPP__
#define __ANOLE_OBJECTS_ENUM_HPP__

#include "object.hpp"

namespace anole
{
class Scope;

class EnumObject : public Object
{
  public:
    EnumObject(SPtr<Scope> scope);

    SPtr<Scope> &scope();

  public:
    Address load_member(const String &name) override;

    void collect(std::function<void(Scope *)>) override;

  private:
    SPtr<Scope> scope_;
};
}

#endif
