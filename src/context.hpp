#pragma once

#include <map>
#include <stack>
#include <string>
#include <filesystem>
#include "code.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "helper.hpp"

namespace anole
{
// Context should be contructed by make_shared
class Context : public std::enable_shared_from_this<Context>
{
  public:
    using StackType = std::stack<Address>;

    // this for resume from ContObject
    Context(SPtr<Context> resume)
      : pre_context_(resume->pre_context_)
      , scope_(std::make_shared<Scope>(resume->scope_))
      , code_(resume->code_), pc_(resume->pc_)
      , stack_(std::make_shared<StackType>(*resume->stack_))
      , current_path_(resume->current_path_) {}

    // copy ctor
    Context(const Context &context)
      : pre_context_(context.pre_context_)
      , scope_(context.scope_)
      , code_(context.code_), pc_(context.pc_)
      , stack_(std::make_shared<StackType>(*context.stack_))
      , current_path_(context.current_path_) {}

    Context(SPtr<Code> code,
        std::filesystem::path path = std::filesystem::current_path())
      : pre_context_(nullptr)
      , scope_(std::make_shared<Scope>(nullptr))
      , code_(code), pc_(0)
      , stack_(std::make_shared<StackType>())
      , current_path_(std::move(path)) {}

    Context(SPtr<Context> pre, SPtr<Scope> scope,
        SPtr<Code> code, std::size_t pc = 0)
      : pre_context_(pre)
      , scope_(std::make_shared<Scope>(scope))
      , code_(std::move(code)), pc_(pc)
      , stack_(pre->stack_)
      , current_path_(pre->current_path_) {}

    static void set_args(int argc, char *argv[], int start);
    static const std::vector<char *> &get_args();

    static void execute();

    static void
    add_not_defined_symbol(
        const std::string &name,
        const Address &ptr);

    static void
    rm_not_defined_symbol(const Address &ptr);

    static const std::string
    &get_not_defined_symbol(const Address &ptr);

    SPtr<Context> &pre_context()
    {
        return pre_context_;
    }

    SPtr<Scope> &scope()
    {
        return scope_;
    }

    SPtr<Code> &code()
    {
        return code_;
    }

    std::size_t &pc()
    {
        return pc_;
    }

    Instruction &ins()
    {
        return code_->ins_at(pc_);
    }

    Instruction &ins_at(std::size_t index)
    {
        return code_->ins_at(index);
    }

    Opcode &opcode()
    {
        return code_->opcode_at(pc_);
    }

    std::any &oprand()
    {
        return code_->oprand_at(pc_);
    }

    void push(ObjectPtr value)
    {
        stack_->push(std::make_shared<ObjectPtr>(std::move(value)));
    }

    void push_address(Address ptr)
    {
        stack_->push(move(ptr));
    }

    template<typename R = Object>
    R *top()
    {
        if (*stack_->top() == nullptr)
        {
            throw RuntimeError(
                "no such var named " +
                get_not_defined_symbol(stack_->top()));
        }
        return reinterpret_cast<R *>(stack_->top().get()->get());
    }

    Address &top_address()
    {
        return stack_->top();
    }

    void set_top(ObjectPtr ptr)
    {
        stack_->top() = std::make_shared<ObjectPtr>(std::move(ptr));
    }

    ObjectPtr pop()
    {
        // use copy ctor,
        // because the object ptr may be pointed by other address
        auto res = *stack_->top();
        stack_->pop();
        return res;
    }

    Address pop_address()
    {
        auto res = std::move(top_address());
        stack_->pop();
        return res;
    }

    std::size_t size() const
    {
        return stack_->size();
    }

    SPtr<StackType> &get_stack()
    {
        return stack_;
    }

    std::filesystem::path &current_path()
    {
        return current_path_;
    }

    void set_call_anchor();
    std::size_t get_call_args_num();

    void set_return_anchor();
    std::size_t get_return_vals_num();

  private:
    SPtr<Context> pre_context_;
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    std::size_t pc_;
    SPtr<StackType> stack_;
    std::filesystem::path current_path_;
};

extern SPtr<Context> theCurrentContext;
}
