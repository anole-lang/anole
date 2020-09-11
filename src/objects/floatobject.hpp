#pragma once

#include "object.hpp"

namespace anole
{
class FloatObject : public Object
{
  public:
    constexpr FloatObject(double value) noexcept
      : Object(ObjectType::Float)
      , value_(value)
    {
        // ...
    }

    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    Object *neg() override;
    Object *add(Object *) override;
    Object *sub(Object *) override;
    Object *mul(Object *) override;
    Object *div(Object *) override;
    Object *ceq(Object *) override;
    Object *cne(Object *) override;
    Object *clt(Object *) override;
    Object *cle(Object *) override;

    constexpr double value() const noexcept
    {
        return value_;
    }

  private:
    double value_;
};
}
