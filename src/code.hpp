#pragma once

#include <list>
#include <memory>
#include <type_traits>
#include "operation.hpp"

namespace ice_language
{
class Code
{
  public:
    template <typename Op, typename ...Args>
    void add_op(Args ...args)
    {
        if constexpr (sizeof...(Args) == 0)
        {
            operations_.push_back(std::make_shared<Op>());
        }
        else
        {
            std::vector<std::shared_ptr<void>> oprands;
            helper(oprands, args);
        }
    }

    std::list<std::shared_ptr<Operation>> &get_operations()
    {
        return operations_;
    }

  private:
    template <typename T, typename ...Args>
    void helper(std::vector<std::shared_ptr<void>> &oprands,
        T value)
    {
        if (std::is_same<T, nullptr>::value)
        {
            oprands.push_back(nullptr);
        }
        else
        {
            oprands.push_back(make_shared<T>(value));
        }
    }

    std::list<std::shared_ptr<Operation>> operations_;
};
}
