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
    void push(std::shared_ptr<void> value)
    {
        stack_.push(value);
    }
    template <typename R>
    std::shared_ptr<R> top()
    {
        return reinterpret_pointer_cast<R>(stack_.top());
    }
    template <typename R>
    std::shared_ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }

    void execute_op(Operation &ins);
    void execute_code(Code &code);

  private:

    std::stack<std::shared_ptr<void>> stack_;
};
} // namespace ice_language
