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
    ObjectPtr neg() override;
    ObjectPtr add(ObjectPtr) override;
    ObjectPtr sub(ObjectPtr) override;
    ObjectPtr mul(ObjectPtr) override;
    ObjectPtr div(ObjectPtr) override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    ObjectPtr clt(ObjectPtr) override;
    ObjectPtr cle(ObjectPtr) override;

    constexpr double value() const noexcept
    {
        return value_;
    }

  private:
    double value_;
};
}
