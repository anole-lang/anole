#pragma once

#include <stack>
#include <memory>
#include "code.hpp"

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
        return std::reinterpret_pointer_cast<R>(stack_.top());
    }
    template <typename R>
    std::shared_ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }

    void execute_op(Operation &op)
    {
        op.execute(*this);
    }

    void execute_code(Code &code)
    {
        for (auto op : code.get_operations())
        {
            op->execute(*this);
        }
    }

  private:
    std::stack<std::shared_ptr<void>> stack_;
};
} // namespace ice_language
