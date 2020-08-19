#pragma once

#include "object.hpp"

#include <memory>

namespace anole
{
class BoolObject : public Object
{
  public:
    static BoolObject *the_true();
    static BoolObject *the_false();

    constexpr BoolObject(bool value) noexcept
      : Object(ObjectType::Boolean)
      , value_(value)
    {
        // ...
    }

    bool to_bool() override;
    String to_str() override;

  private:
    bool value_;
};
}
