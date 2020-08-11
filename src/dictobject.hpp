#pragma once

#include "object.hpp"

#include <map>

namespace anole
{
class DictObject : public Object, public std::enable_shared_from_this<DictObject>
{
  public:
    struct ObjectCmp
    {
        bool operator()(const ObjectPtr &lhs, const ObjectPtr &rhs) const
        {
            return lhs->to_key() < rhs->to_key();
        }
    };

    using DataType = std::map<ObjectPtr, Address, ObjectCmp>;

    DictObject()
      : Object(ObjectType::Dict) {}

    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    Address index(ObjectPtr) override;
    Address load_member(const String &name) override;

    DataType &data();
    void insert(ObjectPtr key, ObjectPtr value);

  private:
    DataType data_;
};
}
