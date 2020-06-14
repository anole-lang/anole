#pragma once

#include <vector>
#include <string>
#include <functional>
#include "object.hpp"

#define REGISTER_BUILTIN(NAME, FUNC) \
    __attribute__((constructor)) static void F_##NAME () \
    { \
        BuiltInFunctionObject::register_built_in_function(#NAME, \
            [](std::size_t n) \
                FUNC \
        ); \
    }

namespace anole
{
class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::function<void(std::size_t)> func)
      : Object(ObjectType::BuiltinFunc)
      , func_(std::move(func)) {}

    std::string to_str() override;
    void call(std::size_t num) override;
    void call_tail(std::size_t num) override;

    static ObjectPtr load_built_in_function(const std::string &);
    static void register_built_in_function(const std::string &,
        std::function<void(std::size_t)>);

  private:
    std::function<void(std::size_t)> func_;
};
}
