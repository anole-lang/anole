#ifndef __ANOLE_OBJECTS_NONE_HPP__
#define __ANOLE_OBJECTS_NONE_HPP__

#include "object.hpp"

namespace anole
{
class NoneObject : public Object
{
  public:
    static Object *one();

  private:
    constexpr NoneObject() noexcept
      : Object(ObjectType::None)
    {
        // ...
    }
};
}

#endif
