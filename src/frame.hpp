#pragma once

#include <map>
#include <stack>
#include <string>
#include "helper.hpp"
#include "code.hpp"
#include "scope.hpp"

namespace ice_language
{
class Frame
{
  public:
    Frame()
      : return_to_(nullptr),
        scope_(std::make_shared<Scope>(nullptr)) {}
    Frame(Ptr<Scope> scope)
      : return_to_(nullptr),
        scope_(scope) {}

    void execute_code(Code &code);

    void set_return_to(Ptr<Frame> return_to)
    {
        // assert return_to_ is not nullptr
        return_to_ = return_to;
    }

    void set_return()
    {
        // assert stack_ is not empty
        // assert return_to_ is not nullptr
        return_to_->push(pop());
    }

    void push(VoidPtr value)
    {
        stack_.push(std::make_shared<VoidPtr>(value));
    }

    void push_straight(Ptr<VoidPtr> ptr)
    {
        stack_.push(ptr);
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

  private:
    Ptr<Frame> return_to_;
    Ptr<Scope> scope_;
    std::stack<Ptr<VoidPtr>> stack_;
};
}
