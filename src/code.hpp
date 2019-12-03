#pragma once

#include <vector>
#include <type_traits>
#include "helper.hpp"
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    template <Op op = Op::PlaceHolder>
    std::size_t add_ins()
    {
        instructions_.push_back({
            op, nullptr
        });
        return instructions_.size() - 1;
    }

    template <Op op, typename T>
    std::size_t add_ins(T value)
    {
        if constexpr (op == Op::Push
            and std::is_same<T, std::nullptr_t>::value)
        {
            instructions_.push_back({
                Op::Push, nullptr
            });
        }
        else
        {
            instructions_.push_back({
                op, std::make_shared<T>(value)
            });
        }
        return instructions_.size() - 1;
    }

    template <Op op>
    void set_ins(std::size_t ind)
    {
        instructions_[ind] = {op, nullptr};
    }

    template <Op op, typename T>
    void set_ins(std::size_t ind, T value)
    {
        if constexpr (op == Op::Push
            and std::is_same<T, std::nullptr_t>::value)
        {
            instructions_[ind] = {
                Op::Push, nullptr
            };
        }
        else
        {
            instructions_[ind] = {
                op, std::make_shared<T>(value)
            };
        }
    }

    std::size_t size()
    {
        return instructions_.size();
    }

    std::vector<Instruction> &get_instructions()
    {
        return instructions_;
    }

  private:

    std::vector<Instruction> instructions_;
};
}
