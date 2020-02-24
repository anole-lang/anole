#pragma once

#include <map>
#include <stack>
#include <string>
#include <exception>
#include <stdexcept>
#include "helper.hpp"
#include "code.hpp"
#include "scope.hpp"

namespace ice_language
{
// Frame should be contructed by make_shared
class Frame : public std::enable_shared_from_this<Frame>
{
  public:
    Frame(Ptr<Code> code) : Frame(nullptr, code) {}
    Frame(Ptr<Scope> scope, Ptr<Code> code)
      : Frame(nullptr, scope, code) {}
    Frame(Ptr<Frame> return_to, Ptr<Scope> scope, Ptr<Code> code, std::size_t pc = 1)
      : return_to_(return_to), scope_(std::make_shared<Scope>(scope)),
        code_(code), pc_(pc) {}

    static void execute();

    Ptr<Frame> return_to()
    {
        return return_to_;
    }

    Ptr<Scope> &scope()
    {
        return scope_;
    }

    Ptr<Code> &code()
    {
        return code_;
    }

    std::size_t &pc()
    {
        return pc_;
    }

    Instruction &ins()
    {
        return code_->get_instructions()[pc_];
    }

    std::any &oprand()
    {
        return code_->oprand_at(pc_);
    }

    void push(ObjectPtr value)
    {
        stack_.push(std::make_shared<ObjectPtr>(value));
    }

    void push_straight(Ptr<ObjectPtr> ptr)
    {
        stack_.push(ptr);
    }

    template <typename R = Object>
    Ptr<R> top()
    {
        if (*stack_.top() == nullptr)
        {
            throw std::runtime_error("no such var");
        }
        return std::reinterpret_pointer_cast<R>(*stack_.top());
    }

    Ptr<ObjectPtr> &top_straight()
    {
        return stack_.top();
    }

    void set_top(ObjectPtr ptr)
    {
        stack_.top() = std::make_shared<ObjectPtr>(ptr);
    }

    template <typename R = Object>
    Ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }

    Ptr<ObjectPtr> pop_straight()
    {
        auto res = top_straight();
        stack_.pop();
        return res;
    }

    std::size_t size()
    {
        return stack_.size();
    }

    bool empty()
    {
        return stack_.empty();
    }

    std::stack<Ptr<ObjectPtr>> &stack()
    {
        return stack_;
    }

  private:
    Ptr<Frame> return_to_;
    Ptr<Scope> scope_;
    Ptr<Code> code_;
    std::size_t pc_;
    std::stack<Ptr<ObjectPtr>> stack_;
};

inline Ptr<Frame> theCurrentFrame = nullptr;
}
