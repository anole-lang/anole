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

namespace ice_language
{
class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::function<void()> func)
      : func_(std::move(func)) {}

    static ObjectPtr load_built_in_function(const std::string &);
    static void register_built_in_function(const std::string &,
        std::function<void()>);

    void operator()();

  private:
    std::function<void()> func_;
};
}
