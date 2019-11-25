#pragma once

#include <memory>
#include <utility>
#include "operation.hpp"

namespace ice_language
{
struct Instruction
{
    Instruction(Opcode opcode,
        std::shared_ptr<OprandsBase> oprands)
      : opcode(opcode), oprands(oprands)
    {}
    Opcode opcode;
    std::shared_ptr<OprandsBase> oprands;
};
}
