#pragma once

#include <string>
#include <utility>
#include "object.hpp"

namespace anole
{
class StringObject : public Object
{
  public:
    StringObject(std::string value)
      : value_(std::move(value)) {}

    bool to_bool() override;
    std::string to_str() override;

    ObjectPtr add(ObjectPtr) override;
    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    ObjectPtr clt(ObjectPtr) override;
    ObjectPtr cle(ObjectPtr) override;
    SPtr<ObjectPtr> index(ObjectPtr) override;
    SPtr<ObjectPtr> load_member(const std::string &name) override;

    const std::string &value() const
    {
        return value_;
    }

  private:
    std::string value_;
};
}
