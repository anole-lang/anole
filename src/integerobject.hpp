#pragma once

#include "object.hpp"

namespace ice_language
{
class IntegerObject : public Object
{
  public:
    IntegerObject(long value)
      : value_(value) {}

    bool to_bool() override;
    std::string to_str() override;
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

    long value() const { return value_; }

  private:
    long value_;
};
}
