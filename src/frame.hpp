#pragma once

#include <map>
#include <stack>
#include <string>
#include "helper.hpp"
#include "code.hpp"

namespace ice_language
{
using VoidPtr = Ptr<void>;

class Frame
{
  public:
    Frame() : Frame(nullptr) {}
    Frame(Ptr<Frame> pre) : return_to_(nullptr), pre_(pre) {}
    Frame(const Frame &scope) : Frame(scope.pre_) {}

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

    VoidPtr find_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
        }
        else
        {
            return pre_
                 ? pre_->find_symbol(name)
                 : nullptr;
        }
    }

    void create_symbol(const std::string &name)
    {
        symbols_[name] = std::make_shared<VoidPtr>(nullptr);
    }

    VoidPtr load_symbol(const std::string &name)
    {
        auto ptr = find_symbol(name);
        return ptr ? ptr : std::make_shared<VoidPtr>(nullptr);
    }

  private:
    Ptr<Frame> return_to_;
    Ptr<Frame> pre_;
    std::stack<Ptr<VoidPtr>> stack_;
    std::map<std::string, Ptr<VoidPtr>> symbols_;
};
}
