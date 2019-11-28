#pragma once

#include <map>
#include <stack>
#include <memory>
#include <string>
#include "helper.hpp"
#include "code.hpp"

namespace ice_language
{
using VoidPtr = Ptr<void>;

class Scope
{
  public:
    Scope(Ptr<Scope> pre) : pre_(pre) {}

    void push(VoidPtr value)
    {
        stack_.push(std::make_shared<VoidPtr>(value));
    }
    template <typename R = void>
    Ptr<R> top()
    {
        return std::reinterpret_pointer_cast<R>(*stack_.top());
    }
    template <typename R = void>
    Ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }
    Ptr<VoidPtr> pop_straight()
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
    Ptr<Scope> pre_;
    std::stack<Ptr<VoidPtr>> stack_;
    std::map<std::string, Ptr<VoidPtr>> symbols_;
};
}
