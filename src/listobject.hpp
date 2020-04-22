#pragma once

#include <list>
#include "object.hpp"

namespace anole
{
class ListObject : public Object
{
  public:
    ListObject()
      : Object(ObjectType::List) {}

    bool to_bool() override;
    std::string to_str() override;
    std::string to_key() override;
    ObjectPtr add(ObjectPtr) override;
    Address index(ObjectPtr) override;
    Address load_member(const std::string &name) override;

    std::list<Address> &objects();
    void append(ObjectPtr obj);

  private:
    std::list<Address> objects_;
};
}
