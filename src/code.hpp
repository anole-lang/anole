#pragma once

#include <list>
#include <type_traits>
#include "helper.hpp"
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    template <typename Ins>
    void add_ins()
    {
        instructions_.push_back(std::make_shared<Ins>());
    }

    template <typename Ins, typename T>
    void add_ins(T value)
    {
        if constexpr (std::is_same<T, std::nullptr_t>::value)
        {
            instructions_.push_back(std::make_shared<Ins>(nullptr));
        }
        else if constexpr (std::is_same<Ins, Push>::value)
        {
            instructions_.push_back(std::make_shared<Push>(std::make_shared<T>(value)));
        }
        else
        {
            instructions_.push_back(std::make_shared<Ins>(value));
        }
    }

    std::list<Ptr<Instruction>> &get_instructions()
    {
        return instructions_;
    }

  private:

    std::list<Ptr<Instruction>> instructions_;
};
}
