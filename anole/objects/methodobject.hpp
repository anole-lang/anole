#ifndef __ANOLE_OBJECTS_METHOD_HPP__
#define __ANOLE_OBJECTS_METHOD_HPP__

#include "object.hpp"

namespace anole
{
class MethodObject : public Object
{
  public:
    MethodObject(Object *callee,
        Object *binded_obj
    );

    void call(Size num) override;

    void collect(std::function<void(Object *)>) override;

  private:
    Object *callee_;
    Object *binded_obj_;
};
} // namespace anole

#endif
