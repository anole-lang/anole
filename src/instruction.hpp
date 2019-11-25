#pragma once

#include <memory>
#include <vector>
#include "operation.hpp"

namespace ice_language
{
struct Instruction
{
    Instruction(Opcode opcode,
        std::vector<std::shared_ptr<void>> oprands = {})
      : opcode(opcode), oprands(oprands)
    {}
    Opcode opcode;
    std::vector<std::shared_ptr<void>> oprands;
};
}
