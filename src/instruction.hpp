#pragma once

#include <memory>
#include <utility>
#include "operation.hpp"

namespace ice_language
{
struct Instruction
{
    Opcode opcode;
    std::unique_ptr<OprandsBase> oprands;
};
}
