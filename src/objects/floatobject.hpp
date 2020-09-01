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
    ObjectSPtr neg() override;
    ObjectSPtr add(ObjectRawPtr) override;
    ObjectSPtr sub(ObjectRawPtr) override;
    ObjectSPtr mul(ObjectRawPtr) override;
    ObjectSPtr div(ObjectRawPtr) override;
    ObjectSPtr ceq(ObjectRawPtr) override;
    ObjectSPtr cne(ObjectRawPtr) override;
    ObjectSPtr clt(ObjectRawPtr) override;
    ObjectSPtr cle(ObjectRawPtr) override;

    constexpr double value() const noexcept
    {
        return value_;
    }

  private:
    double value_;
};
}
