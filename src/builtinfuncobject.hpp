#pragma once

#include <vector>
#include <string>
#include <functional>
#include "object.hpp"

#define REGISTER_BUILTIN(NAME, ARGS_NUM, FUNC) \
    __attribute__((constructor)) static void F_##NAME () \
    { \
        BuiltInFunctionObject::register_built_in_function(#NAME, ARGS_NUM, \
            [](vector<ObjectPtr> args) -> ObjectPtr \
                FUNC \
        ); \
    }

namespace ice_language
{
class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::size_t args_num,
        std::function<ObjectPtr(std::vector<ObjectPtr>&)> func)
      : args_num_(args_num),
        func_(std::move(func)) {}

    static ObjectPtr load_built_in_function(const std::string &);
    static void register_built_in_function(
        const std::string &, std::size_t,
        std::function<ObjectPtr(std::vector<ObjectPtr>&)>);

    void operator()(std::shared_ptr<class Frame> frame);

  private:
    std::size_t args_num_;
    std::function<ObjectPtr(std::vector<ObjectPtr>&)> func_;
};
}
