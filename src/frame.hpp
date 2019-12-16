#pragma once

#include <map>
#include <stack>
#include <string>
#include "helper.hpp"
#include "code.hpp"
#include "scope.hpp"

namespace ice_language
{
// Frame should be contructed by make_shared
class Frame : public std::enable_shared_from_this<Frame>
{
  public:
    Frame() : Frame(std::make_shared<Scope>(nullptr)) {}
    Frame(Ptr<Scope> scope) : Frame(nullptr, scope) {}
    Frame(Ptr<Frame> return_to, Ptr<Scope> scope)
      : return_to_(return_to), scope_(scope) {}

    void execute_code(Code &code, std::size_t base = 0);

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
        return std::reinterpret_pointer_cast<R>(*stack_.top());
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
        auto res = stack_.top();
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

  private:
    Ptr<Frame> return_to_;
    Ptr<Scope> scope_;
    std::stack<Ptr<ObjectPtr>> stack_;
};

}
