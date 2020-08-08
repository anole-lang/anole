#pragma once

#include <memory>
#include "object.hpp"

namespace anole
{
class BoolObject : public Object
{
  public:
    constexpr BoolObject(bool value) noexcept
      : Object(ObjectType::Boolean)
      , value_(value) {}

    bool to_bool() override;
    String to_str() override;

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;

  private:
    bool value_;
};

inline auto theTrue = std::make_shared<BoolObject>(true);
inline auto theFalse = std::make_shared<BoolObject>(false);
}
