#pragma once

#include "object.hpp"

namespace anole
{
class NoneObject : public Object
{
  public:
    constexpr NoneObject() noexcept
      : Object(ObjectType::None)
    {
        // ...
    }

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
};

inline auto theNone = std::make_shared<NoneObject>();
}
