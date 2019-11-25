#pragma once

#include <stack>
#include <memory>
#include "operation.hpp"

namespace ice_language
{
class Instruction;
class Code;

class VM
{
  public:
    void execute_ins(Instruction &ins);
    void execute_code(Code &code);

  private:
    void push(Instruction &ins);
    void add();

    std::stack<std::shared_ptr<void>> stack_;
};
} // namespace ice_language
