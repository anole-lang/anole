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
    void sub();

    template <typename R>
    std::shared_ptr<R> pop()
    {
        auto res = reinterpret_pointer_cast<R>(stack_.top());
        stack_.pop();
        return res;
    }

    std::stack<std::shared_ptr<void>> stack_;
};
} // namespace ice_language
