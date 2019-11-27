#pragma once

#include <map>
#include <stack>
#include <memory>
#include <string>
#include "code.hpp"

namespace ice_language
{
class Scope
{
    using VoidPtr = std::shared_ptr<void>;
  public:
    Scope(std::shared_ptr<Scope> pre) : pre_(pre) {}

    void push(VoidPtr value)
    {
        stack_.push(std::make_shared<VoidPtr>(value));
    }
    template <typename R = void>
    std::shared_ptr<R> top()
    {
        return std::reinterpret_pointer_cast<R>(*stack_.top());
    }
    template <typename R = void>
    std::shared_ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }
    std::shared_ptr<VoidPtr> pop_straight()
    {
        auto res = stack_.top();
        stack_.pop();
        return res;
    }

    VoidPtr load_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
        }
        else
        {
            return pre_
                 ? pre_->load_symbol(name)
                 : std::make_shared<VoidPtr>(nullptr);
        }
    }

    void execute_ins(Instruction &ins)
    {
        ins.execute(*this);
    }

    void execute_code(Code &code)
    {
        for (auto ins : code.get_instructions())
        {
            ins->execute(*this);
        }
    }

  private:
    std::shared_ptr<Scope> pre_;
    std::stack<std::shared_ptr<VoidPtr>> stack_;
    std::map<std::string, std::shared_ptr<VoidPtr>> symbols_;
};
}
