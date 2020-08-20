#pragma once

#include "object.hpp"

#include <vector>
#include <functional>

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
    friend class Collector;

    BuiltInFunctionObject(std::function<void(Size)> func)
      : Object(ObjectType::BuiltinFunc)
      , func_(std::move(func))
    {
        // ...
    }

    String to_str() override;
    void call(Size num) override;

    static ObjectSPtr load_built_in_function(const String &);

    static void register_built_in_function(
        const String &,
        std::function<void(Size)>
    );

  private:
    std::function<void(Size)> func_;
};
}
