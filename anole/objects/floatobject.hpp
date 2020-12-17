#ifndef __ANOLE_OBJECTS_FLOAT_HPP__
#define __ANOLE_OBJECTS_FLOAT_HPP__

#include "object.hpp"

namespace anole
{
class FloatObject : public Object
{
  public:
    constexpr FloatObject(double value) noexcept
      : Object(ObjectType::Float), value_(value)
    {
        // ...
    }

    constexpr double value() const noexcept
    {
        return value_;
    }

  public:
    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    Object *neg() override;
    Object *add(Object *) override;
    Object *sub(Object *) override;
    Object *mul(Object *) override;
    Object *div(Object *) override;
    Object *ceq(Object *) override;
    Object *cne(Object *) override;
    Object *clt(Object *) override;
    Object *cle(Object *) override;

  private:
    double value_;
};
}

#endif
