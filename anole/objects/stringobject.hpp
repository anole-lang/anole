#ifndef __ANOLE_OBJECTS_STRING_HPP__
#define __ANOLE_OBJECTS_STRING_HPP__

#include "object.hpp"

#include <utility>

namespace anole
{
class StringObject : public Object
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

#endif
