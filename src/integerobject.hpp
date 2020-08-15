#pragma once

#include "object.hpp"

namespace anole
{
class IntegerObject : public Object
{
  public:
    constexpr IntegerObject(int64_t value) noexcept
      : Object(ObjectType::Integer)
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
    ObjectPtr mod(ObjectPtr) override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    ObjectPtr clt(ObjectPtr) override;
    ObjectPtr cle(ObjectPtr) override;
    ObjectPtr bneg() override;
    ObjectPtr bor(ObjectPtr) override;
    ObjectPtr bxor(ObjectPtr) override;
    ObjectPtr band(ObjectPtr) override;
    ObjectPtr bls(ObjectPtr) override;
    ObjectPtr brs(ObjectPtr) override;
    Address load_member(const String &name) override;

    constexpr int64_t value() const noexcept
    {
        return value_;
    }

  private:
    int64_t value_;
};
}
