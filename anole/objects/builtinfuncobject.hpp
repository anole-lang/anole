#ifndef __ANOLE_OBJECTS_BUILTINFUNC_HPP__
#define __ANOLE_OBJECTS_BUILTINFUNC_HPP__

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
    static Object *load_built_in_function(const String &);

    static void register_built_in_function(
        const String &,
        std::function<void(Size)>
    );

  public:
    BuiltInFunctionObject(std::function<void(Size)> func,
        Object *bind = nullptr)
      : Object(ObjectType::BuiltinFunc)
      , func_(std::move(func))
      , bind_(bind)
    {
        // ...
    }

    String to_str() override;
    void call(Size num) override;

    void collect(std::function<void(Object *)>) override;

  private:
    std::function<void(Size)> func_;
    Object *bind_;
};
}

#endif
