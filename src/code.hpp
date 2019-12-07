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
    Code(bool interpret_mode = false)
      : interpret_mode_(interpret_mode) {}

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

    void push_break(std::size_t ind)
    {
        breaks_.push_back(ind);
    }

    void set_break_to(std::size_t ind,
        std::size_t base)
    {
        decltype(breaks_) breaks;
        for (auto i : breaks_)
        {
            if (i > base)
            {
                set_ins<Op::Jump>(i, ind);
            }
            else
            {
                breaks.push_back(i);
            }
        }
        breaks_ = breaks;
    }

    void push_continue(std::size_t ind)
    {
        continues_.push_back(ind);
    }

    void set_continue_to(std::size_t ind,
        std::size_t base)
    {
        decltype(continues_) continues;
        for (auto i : continues_)
        {
            if (i > base)
            {
                set_ins<Op::Jump>(i, ind);
            }
            else
            {
                continues.push_back(i);
            }
        }
        continues_ = continues;
    }

    bool check()
    {
        if (!breaks_.empty() && !continues_.empty())
        {
            // throw
            return false;
        }
        return true;
    }

    bool interpret_mode()
    {
        return interpret_mode_;
    }

  private:
    std::vector<Instruction> instructions_;
    // these two should be checked is empty or not
    std::vector<std::size_t> breaks_, continues_;
    bool interpret_mode_;
};
}
