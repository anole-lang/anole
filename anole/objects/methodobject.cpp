#include "objects.hpp"

#include "../runtime/runtime.hpp"

using namespace std;

namespace anole
{
MethodObject::MethodObject(Object *callee,
    Object *binded_obj)
  : Object(ObjectType::Method)
  , callee_(callee)
  , binded_obj_(binded_obj)
{
    // ...
}

void MethodObject::call(Size num)
{
    Context::current()->push(binded_obj_);
    callee_->call(num + 1);
}
} // namespace anole
