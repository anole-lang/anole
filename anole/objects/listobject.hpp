#ifndef __ANOLE_OBJECTS_LIST_HPP__
#define __ANOLE_OBJECTS_LIST_HPP__

#include "object.hpp"

#include <list>

namespace anole
{
class ListObject : public Object
{
  public:
    ListObject() noexcept;

    std::list<Address> &objects();
    void append(Object *ptr);

  public:
    bool to_bool() override;
    String to_str() override;
    String to_key() override;
    Object *add(Object *) override;
    Address index(Object *) override;
    Address load_member(const String &name) override;

    void collect(std::function<void(Object *)>) override;

  private:
    std::list<Address> objects_;
};

class ListIteratorObject : public Object
{
  public:
    ListIteratorObject(ListObject *bind)
      : Object(ObjectType::ListIterator)
      , bind_(bind)
      , current_(bind->objects().begin())
    {
        // ...
    }

    Address load_member(const String &name) override;

    void collect(std::function<void(Object *)>) override;

    bool has_next() { return current_ != bind_->objects().end(); }
    Address next() { return *current_++; }

  private:
    ListObject *bind_;
    std::list<Address>::iterator current_;
};
}

#endif
