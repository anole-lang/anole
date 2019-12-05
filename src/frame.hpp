#pragma once

#include <map>
#include <stack>
#include <string>
#include "helper.hpp"
#include "code.hpp"

namespace ice_language
{
using VoidPtr = Ptr<void>;

using Scope = class Frame;
class Frame
{
  public:
    Frame() : Frame(nullptr) {}
    Frame(Ptr<Scope> pre_scope) : return_to_(nullptr), pre_scope_(pre_scope) {}
    Frame(const Frame &scope) : Frame(scope.pre_scope_) {}

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

    Ptr<VoidPtr> find_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
        }
        else
        {
            return pre_scope_
                 ? pre_scope_->find_symbol(name)
                 : nullptr;
        }
    }

    void create_symbol(const std::string &name)
    {
        if (!symbols_.count(name))
        {
            symbols_[name] = std::make_shared<VoidPtr>(nullptr);
        }
    }

    Ptr<VoidPtr> load_symbol(const std::string &name)
    {
        auto ptr = find_symbol(name);
        return ptr ? ptr
             : (symbols_[name] = std::make_shared<VoidPtr>(nullptr));
    }

  private:
    Ptr<Frame> return_to_;
    Ptr<Scope> pre_scope_;
    std::stack<Ptr<VoidPtr>> stack_;
    std::map<std::string, Ptr<VoidPtr>> symbols_;
};
}
