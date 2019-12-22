#pragma once

#include <vector>
#include <string>
#include <functional>
#include "frame.hpp"
#include "object.hpp"

namespace ice_language
{
ObjectPtr load_built_in_function(std::string);

class BuiltInFunctionObject : public Object
{
  public:
    BuiltInFunctionObject(std::size_t args_num,
        std::function<ObjectPtr(std::vector<ObjectPtr>&)> func)
      : args_num_(args_num),
        func_(std::move(func)) {}

    void operator()(std::shared_ptr<Frame> frame)
    {
        std::vector<ObjectPtr> args;
        for (size_t i = 0; i < args_num_; ++i)
        {
            args.push_back(frame->pop());
        }
        auto res = func_(args);
        if (res)
        {
            frame->push(res);
        }
    }

  private:
    std::size_t args_num_;
    std::function<ObjectPtr(std::vector<ObjectPtr>&)> func_;
};
}
