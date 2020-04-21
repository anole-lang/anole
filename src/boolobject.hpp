#pragma once

#include <memory>
#include "object.hpp"

namespace anole
{
class BoolObject : public Object
{
  public:
    BoolObject(bool value)
      : Object(ObjectType::Boolean)
      , value_(value) {}

    bool to_bool() override;
    std::string to_str() override;

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;

  private:
    bool value_;
};

inline auto theTrue = std::make_shared<BoolObject>(true);
inline auto theFalse = std::make_shared<BoolObject>(false);
}
