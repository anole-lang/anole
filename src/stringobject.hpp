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

    ObjectSPtr add(ObjectRawPtr) override;
    ObjectSPtr ceq(ObjectRawPtr) override;
    ObjectSPtr cne(ObjectRawPtr) override;
    ObjectSPtr clt(ObjectRawPtr) override;
    ObjectSPtr cle(ObjectRawPtr) override;
    Address index(ObjectSPtr) override;
    Address load_member(const String &name) override;

    const String &value() const
    {
        return value_;
    }

  private:
    String value_;
};
}
