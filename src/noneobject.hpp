#pragma once

#include <memory>
#include "object.hpp"

namespace ice_language
{
class NoneObject : public Object
{
  public:
    NoneObject() = default;
};

inline auto theNone = std::make_shared<NoneObject>();
}
