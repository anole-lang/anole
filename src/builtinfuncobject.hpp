#pragma once

#include <vector>
#include <string>
#include <functional>
#include "object.hpp"

#define REGISTER_BUILTIN(NAME, FUNC) \
    __attribute__((constructor)) static void F_##NAME () \
    { \
        BuiltInFunctionObject::register_built_in_function(#NAME, \
            [] \
                FUNC \
        ); \
    }

namespace anole
{
class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::function<void()> func)
      : Object(ObjectType::BuiltinFunc)
      , func_(std::move(func)) {}

    std::string to_str() override;
    void call() override;
    void call_tail() override;

    static ObjectPtr load_built_in_function(const std::string &);
    static void register_built_in_function(const std::string &,
        std::function<void()>);

  private:
    std::function<void()> func_;
};
}
