#pragma once

#include "object.hpp"

namespace anole
{
class NoneObject : public Object
{
  public:
    static Object *one();

  private:
    constexpr NoneObject() noexcept
      : Object(ObjectType::None)
    {
        // ...
    }
};
}
