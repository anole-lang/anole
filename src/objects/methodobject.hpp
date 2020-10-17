#pragma once

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

  private:
    Object *callee_;
    Object *binded_obj_;
};
} // namespace anole
