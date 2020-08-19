#pragma once

#include "object.hpp"
#include "collector.hpp"

#include <list>

namespace anole
{
class ListObject : public Object
{
  public:
    ListObject() : Object(ObjectType::List) {}

    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    Object *add(Object *) override;
    Address index(Object *) override;
    Address load_member(const String &name) override;

    std::list<Address> &objects();
    void append(Object *obj);

  private:
    std::list<Address> objects_;
};

class ListIteratorObject : public Object, public EnableCollect<Object>
{
  public:
    friend class Collector;

    ListIteratorObject(ListObject *bind)
      : Object(ObjectType::ListIterator)
      , EnableCollect(bind), bind_(bind)
      , current_(bind->objects().begin())
    {
        // ...
    }

    Address load_member(const String &name) override;

    bool has_next() { return current_ != bind_->objects().end(); }
    Address next() { return *current_++; }

  private:
    ListObject *bind_;
    std::list<Address>::iterator current_;
};
}
