#pragma once

#include <vector>
#include <functional>
#include "object.hpp"

#define REGISTER_BUILTIN(NAME, FUNC) \
    __attribute__((constructor)) static void F_##NAME () \
    { \
        BuiltInFunctionObject::register_built_in_function(#NAME, \
            [](Size n) \
                FUNC \
        ); \
    }

namespace anole
{
class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::function<void(Size)> func)
      : Object(ObjectType::BuiltinFunc)
      , func_(std::move(func)) {}

    String to_str() override;
    void call(Size num) override;

    static ObjectPtr load_built_in_function(const String &);
    static void register_built_in_function(const String &,
        std::function<void(Size)>);

  private:
    std::function<void(Size)> func_;
};
}
