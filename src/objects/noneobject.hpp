#pragma once

#include "object.hpp"

namespace anole
{
class NoneObject : public Object
{
  public:
    static ObjectSPtr one();

    constexpr NoneObject() noexcept
      : Object(ObjectType::None)
    {
        // ...
    }
};
}
