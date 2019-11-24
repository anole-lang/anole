#pragma once

#include "operation.hpp"
#include <memory>

namespace ice_language
{
class Instruction;
class Code;

class VM
{
  public:
    void execute_ins(Instruction &ins);
    void ececute_code(Code &code);

  private:
    void push(Instruction &ins);
};
} // namespace ice_language
