#pragma once

#include <memory>
#include <utility>
#include "operation.hpp"

namespace ice_language
{
struct Instruction
{
    Instruction(Opcode opcode,
        std::unique_ptr<OprandsBase> oprands)
      : opcode(opcode), oprands(std::move(oprands))
    {}
    Opcode opcode;
    std::unique_ptr<OprandsBase> oprands;
};
}
