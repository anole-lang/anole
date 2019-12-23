#pragma once

#include <memory>
#include "object.hpp"

namespace ice_language
{
class BoolObject : public Object
{
  public:
    BoolObject(bool value) : value_(value) {}

  private:
    bool value_;
};

inline auto theTrue = std::make_shared<BoolObject>(true);
inline auto theFalse = std::make_shared<BoolObject>(false);
}
