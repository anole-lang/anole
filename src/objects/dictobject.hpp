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
        bool operator()(const ObjectSPtr &lhs, const ObjectSPtr &rhs) const
        {
            return lhs->to_key() < rhs->to_key();
        }
    };

    /**
     * TODO:
     * ObjectSPtr as key may cause circular reference
    */
    using DataType = std::map<ObjectSPtr, Address, ObjectCmp>;

    DictObject() : Object(ObjectType::Dict)
    {
        // ...
    }

    bool to_bool() override;
    String to_str() override;
    String to_key() override;

    Address index(ObjectSPtr) override;
    Address load_member(const String &name) override;

    void collect(std::function<void(Variable *)>) override;

    DataType &data();
    void insert(ObjectSPtr key, ObjectSPtr value);

  private:
    DataType data_;
};
}
