#pragma once

#include <list>
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    void add_opcode(Opcode opcode);
    void add_common_ins(Instruction &&ins);

  private:
    std::list<Instruction> instructions_;
};
}
