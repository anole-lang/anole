#pragma once

#include <list>
#include "object.hpp"

namespace anole
{
class ListObject : public Object, public std::enable_shared_from_this<ListObject>
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

class ListIteratorObject : public Object, public std::enable_shared_from_this<ListIteratorObject>
{
  public:
    ListIteratorObject(SPtr<ListObject> bind)
      : bind_(bind), current_(bind->objects().begin()) {}

    Address load_member(const std::string &name) override;

    bool has_next() { return current_ != bind_->objects().end(); }
    Address next() { return *current_++; }

  private:
    SPtr<ListObject> bind_;
    std::list<Address>::iterator current_;
};
}
