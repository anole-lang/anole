#pragma once

#include "object.hpp"

#include <utility>

namespace anole
{
class StringObject : public Object, public std::enable_shared_from_this<StringObject>
{
  public:
    StringObject(String value)
      : Object(ObjectType::String)
      , value_(std::move(value))
    {
        // ...
    }

    bool to_bool() override;
    String to_str() override;
    String to_key() override;

    Object *add(Object *) override;
    Object *ceq(Object *) override;
    Object *cne(Object *) override;
    Object *clt(Object *) override;
    Object *cle(Object *) override;
    Address index(Object *) override;
    Address load_member(const String &name) override;

    const String &value() const
    {
        return value_;
    }

  private:
    String value_;
};
}
