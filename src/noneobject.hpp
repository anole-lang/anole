#pragma once

#include <memory>
#include "object.hpp"

namespace ice_language
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
