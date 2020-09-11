#pragma once

#include "object.hpp"

#include <memory>

namespace anole
{
class BoolObject : public Object
{
  public:
    static Object *the_true();
    static Object *the_false();

  public:
    bool to_bool() override;
    String to_str() override;

  private:
    constexpr BoolObject(bool value) noexcept
      : Object(ObjectType::Boolean)
      , value_(value)
    {
        // ...
    }

  private:
    bool value_;
};
}
