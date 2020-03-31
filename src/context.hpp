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
// Context should be contructed by make_shared
class Context : public std::enable_shared_from_this<Context>
{
  private:
    using StackType = std::stack<Ptr<ObjectPtr>>;

  public:
    // this for resume from ContObject
    Context(Ptr<Context> resume)
      : pre_context_(resume->pre_context_),
        scope_(std::make_shared<Scope>(resume->scope_)),
        code_(resume->code_), pc_(resume->pc_),
        stack_(std::make_shared<StackType>(*resume->stack_)) {}

    // copy ctor
    Context(const Context &context)
      : pre_context_(context.pre_context_),
        scope_(context.scope_),
        code_(context.code_), pc_(context.pc_),
        stack_(std::make_shared<StackType>(*context.stack_)) {}

    Context(Ptr<Code> code)
      : pre_context_(nullptr),
        scope_(std::make_shared<Scope>(nullptr)),
        code_(code), pc_(1),
        stack_(std::make_shared<StackType>()) {}

    // the default value of pc is 1 because first ins is PlaceHolder
    Context(Ptr<Context> pre, Ptr<Scope> scope,
        Ptr<Code> code, std::size_t pc = 1)
      : pre_context_(pre), scope_(std::make_shared<Scope>(scope)),
        code_(code), pc_(pc), stack_(pre->stack_) {}

    static void execute();

    Ptr<Context> pre_context()
    {
        return pre_context_;
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

    Instruction &ins_at(std::size_t index) {
        return code_->get_instructions()[index];
    }

    Opcode opcode()
    {
        return code_->opcode_at(pc_);
    }

    std::any &oprand()
    {
        return code_->oprand_at(pc_);
    }

    void push(ObjectPtr value)
    {
        stack_->push(std::make_shared<ObjectPtr>(value));
    }

    void push_straight(Ptr<ObjectPtr> ptr)
    {
        stack_->push(ptr);
    }

    template <typename R = Object>
    Ptr<R> top()
    {
        if (*stack_->top() == nullptr)
        {
            throw std::runtime_error("no such var");
        }
        return std::reinterpret_pointer_cast<R>(*stack_->top());
    }

    Ptr<ObjectPtr> &top_straight()
    {
        return stack_->top();
    }

    void set_top(ObjectPtr ptr)
    {
        stack_->top() = std::make_shared<ObjectPtr>(ptr);
    }

    template <typename R = Object>
    Ptr<R> pop()
    {
        auto res = top<R>();
        stack_->pop();
        return res;
    }

    Ptr<ObjectPtr> pop_straight()
    {
        auto res = top_straight();
        stack_->pop();
        return res;
    }

    std::size_t size()
    {
        return stack_->size();
    }

  private:
    Ptr<Context> pre_context_;
    Ptr<Scope> scope_;
    Ptr<Code> code_;
    std::size_t pc_;
    Ptr<StackType> stack_;
};

inline Ptr<Context> theCurrentContext = nullptr;
}
