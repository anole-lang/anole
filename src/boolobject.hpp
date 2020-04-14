#pragma once

#include <memory>
#include "object.hpp"

namespace anole
{
class BoolObject : public Object
{
  public:
    BoolObject(bool value) : value_(value) {}

    bool to_bool() override;
    std::string to_str() override;

  private:
    bool value_;
};

inline auto theTrue = std::make_shared<BoolObject>(true);
inline auto theFalse = std::make_shared<BoolObject>(false);
}
