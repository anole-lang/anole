#pragma once

#include <string>
#include <utility>
#include "object.hpp"

namespace ice_language
{
class StringObject : public Object
{
  public:
    StringObject(std::string value)
      : value_(std::move(value)) {}

  private:
    std::string value_;
};
}
