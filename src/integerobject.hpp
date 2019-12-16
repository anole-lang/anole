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
    ObjectPtr add(ObjectPtr) override;
    ObjectPtr sub(ObjectPtr) override;
    ObjectPtr mul(ObjectPtr) override;

  private:
    long value_;
};
}
