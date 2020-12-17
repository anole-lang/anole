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
    ListIteratorObject(ListObject *bind);

    bool has_next();
    Address next();

  public:
    Address load_member(const String &name) override;

    void collect(std::function<void(Object *)>) override;

  private:
    ListObject *bind_;
    std::list<Address>::iterator current_;
};
}

#endif
