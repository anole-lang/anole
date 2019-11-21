#pragma once

#include <list>
#include <memory>

namespace ice_language
{
enum class Opcode;

class Instruction
{
  public:
    Instruction() = default;
};

class Code
{
  public:
    Code() = default;
    void append(std::unique_ptr<Instruction> instruction);

  private:
    std::list<std::unique_ptr<Instruction>> instructions_;
};
}
