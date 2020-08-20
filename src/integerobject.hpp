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
    ObjectSPtr neg() override;
    ObjectSPtr add(ObjectRawPtr) override;
    ObjectSPtr sub(ObjectRawPtr) override;
    ObjectSPtr mul(ObjectRawPtr) override;
    ObjectSPtr div(ObjectRawPtr) override;
    ObjectSPtr mod(ObjectRawPtr) override;
    ObjectSPtr ceq(ObjectRawPtr) override;
    ObjectSPtr cne(ObjectRawPtr) override;
    ObjectSPtr clt(ObjectRawPtr) override;
    ObjectSPtr cle(ObjectRawPtr) override;
    ObjectSPtr bneg() override;
    ObjectSPtr bor(ObjectRawPtr) override;
    ObjectSPtr bxor(ObjectRawPtr) override;
    ObjectSPtr band(ObjectRawPtr) override;
    ObjectSPtr bls(ObjectRawPtr) override;
    ObjectSPtr brs(ObjectRawPtr) override;
    Address load_member(const String &name) override;

    constexpr int64_t value() const noexcept
    {
        return value_;
    }

  private:
    int64_t value_;
};
}
