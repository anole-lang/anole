#include "objects.hpp"

#include "../runtime/runtime.hpp"

namespace anole
{
MethodObject::MethodObject(Object *callee, Object *binded_obj)
  : Object(ObjectType::Method), callee_(callee), binded_obj_(binded_obj)
{
    // ...
}

void MethodObject::call(Size num)
{
    theCurrContext->push(binded_obj_);
    callee_->call(num + 1);
}

void MethodObject::collect(std::function<void(Object *)> func)
{
    func(callee_);
    func(binded_obj_);
}
} // namespace anole
