#pragma once

#include "object.hpp"

namespace ice_language
{
class FloatObject : public Object
{
  public:
    FloatObject(double value) : value_(value) {}

    bool to_bool() override;

  private:
    double value_;
};
}
