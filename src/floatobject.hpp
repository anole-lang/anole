#pragma once

#include "object.hpp"

namespace ice_language
{
class FloatObject : public Object
{
  public:
    FloatObject(double value) : value_(value) {}

    bool to_bool() override;
    std::string to_str() override;
    std::string to_key() override;
    ObjectPtr neg() override;
    ObjectPtr add(ObjectPtr) override;
    ObjectPtr sub(ObjectPtr) override;
    ObjectPtr mul(ObjectPtr) override;
    ObjectPtr div(ObjectPtr) override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    ObjectPtr clt(ObjectPtr) override;
    ObjectPtr cle(ObjectPtr) override;

  private:
    double value_;
};
}
