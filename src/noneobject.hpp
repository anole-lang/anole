#pragma once

#include "object.hpp"

namespace anole
{
class NoneObject : public Object
{
  public:
    NoneObject() = default;

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
};

inline auto theNone = std::make_shared<NoneObject>();
}
