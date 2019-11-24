#pragma once

#include <list>
#include <utility>
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    void add_ins(Instruction &&ins)
    {
        instructions_.emplace_back(std::move(ins));
    }

    std::list<Instruction> &get_instructions()
    {
        return instructions_;
    }

  private:
    std::list<Instruction> instructions_;
};
}
