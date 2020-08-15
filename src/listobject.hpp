#pragma once

#include "object.hpp"

#include <list>

namespace anole
{
class ListObject : public Object, public std::enable_shared_from_this<ListObject>
{
  public:
    ListObject() : Object(ObjectType::List) {}

    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    ObjectPtr add(ObjectPtr) override;
    Address index(ObjectPtr) override;
    Address load_member(const String &name) override;

    std::list<Address> &objects();
    void append(ObjectPtr obj);

  private:
    std::list<Address> objects_;
};

class ListIteratorObject : public Object, public std::enable_shared_from_this<ListIteratorObject>
{
  public:
    ListIteratorObject(SPtr<ListObject> bind)
      : Object(ObjectType::ListIterator)
      , bind_(bind), current_(bind->objects().begin())
    {
        // ...
    }

    Address load_member(const String &name) override;

    bool has_next() { return current_ != bind_->objects().end(); }
    Address next() { return *current_++; }

  private:
    SPtr<ListObject> bind_;
    std::list<Address>::iterator current_;
};
}
