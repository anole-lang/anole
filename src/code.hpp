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
    template <typename Ins = void>
    std::size_t add_ins()
    {
        if constexpr (std::is_same(Ins, void)::value)
        {
            instructions_.push_back(nullptr);
        }
        else
        {
            instructions_.push_back(std::make_shared<Ins>());
        }
        return instructions_.size() - 1;
    }

    template <typename Ins, typename T>
    std::size_t add_ins(T value)
    {
        if constexpr (std::is_same<Ins, Push>::value)
        {
            if constexpr (std::is_same<T, std::nullptr_t>::value)
            {
                instructions_.push_back(
                    std::make_shared<Ins>(nullptr)
                );
            }
            else
            {
                instructions_.push_back(
                    std::make_shared<Push>(
                        std::make_shared<T>(value)
                    )
                );
            }
        }
        else
        {
            instructions_.push_back(
                std::make_shared<Ins>(value)
            );
        }
        return instructions_.size() - 1;
    }

    template <typename Ins>
    void set_ins(std::size_t ind)
    {
        instructions_.push_back(std::make_shared<Ins>());
    }

    template <typename Ins, typename T>
    void set_ins(std::size_t ind, T value)
    {
        if constexpr (std::is_same<Ins, Push>::value)
        {
            if constexpr (std::is_same<T, std::nullptr_t>::value)
            {
                instructions_[ind] = std::make_shared<Ins>(nullptr);
            }
            else
            {
                instructions_[ind] = std::make_shared<Push>(
                    std::make_shared<T>(value)
                );
            }
        }
        else
        {
            instructions_[ind] = std::make_shared<Ins>(value);
        }
    }

    std::size_t size()
    {
        return instructions_.size();
    }

    std::vector<Ptr<Instruction>> &get_instructions()
    {
        return instructions_;
    }

  private:

    std::vector<Ptr<Instruction>> instructions_;
};
}
