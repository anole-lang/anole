#pragma once

#include <memory>
#include "code.hpp"

namespace ice_language
{
class VM
{
  public:
    void execute_ins(Instruction &ins);
    void ececute_code(Code &code);
};
} // namespace ice_language
