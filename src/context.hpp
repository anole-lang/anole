#pragma once

#include "base.hpp"
#include "code.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "allocator.hpp"

#include <map>
#include <list>
#include <filesystem>

namespace anole
{
// Context should be contructed by make_shared
class Context
{
    friend class Collector;

  public:
    using Stack = std::list<Address>;

  public:
    static SPtr<Context> &current();

    static void set_args(int argc, char *argv[], int start);
    static const std::vector<char *> &get_args();

    static void execute();

    static void
    add_not_defined_symbol(
        const String &name,
        const Address addr
    );

    static void
    rm_not_defined_symbol(const Address addr);

    static const String
    &get_not_defined_symbol(const Address addr);

  public:
    // this for resume from ContObject
    Context(SPtr<Context> resume)
      : pre_context_(resume->pre_context_)
      , scope_(std::make_shared<Scope>(resume->scope_))
      , code_(resume->code_), pc_(resume->pc_)
      , stack_(std::make_shared<Stack>(*resume->stack_))
      , current_path_(resume->current_path_)
    {
        // ...
    }

    // copy ctor
    Context(const Context &context)
      : pre_context_(context.pre_context_)
      , scope_(context.scope_)
      , code_(context.code_), pc_(context.pc_)
      , stack_(std::make_shared<Stack>(*context.stack_))
      , current_path_(context.current_path_)
    {
        // ...
    }

    Context(SPtr<Code> code,
        std::filesystem::path path = std::filesystem::current_path())
      : pre_context_(nullptr)
      , scope_(std::make_shared<Scope>(nullptr))
      , code_(code), pc_(0)
      , stack_(std::make_shared<Stack>())
      , current_path_(std::move(path))
    {
        // ...
    }

    Context(SPtr<Context> pre, SPtr<Scope> scope,
        SPtr<Code> code, Size pc = 0)
      : pre_context_(pre)
      , scope_(std::make_shared<Scope>(scope))
      , code_(std::move(code)), pc_(pc)
      , stack_(pre->stack_)
      , current_path_(pre->current_path_)
    {
        // ...
    }

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

    Size &pc()
    {
        return pc_;
    }

    Instruction &ins()
    {
        return code_->ins_at(pc_);
    }

    Instruction &ins_at(Size index)
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

    void push(ObjectSPtr sptr)
    {
        stack_->push_back(Allocator<Variable>::alloc(move(sptr)));
    }

    void push_address(Address addr)
    {
        stack_->push_back(addr);
    }

    template<typename R = Object>
    R *top()
    {
        if (stack_->back()->rptr() == nullptr)
        {
            throw RuntimeError(
                "var named " +
                get_not_defined_symbol(stack_->back()) +
                " doesn't reference to any object"
            );
        }
        return reinterpret_cast<R *>(stack_->back()->rptr());
    }

    Address &top_address()
    {
        return stack_->back();
    }

    void set_top(ObjectSPtr sptr)
    {
        stack_->back() = Allocator<Variable>::alloc(move(sptr));
    }

    ObjectSPtr pop()
    {
        auto &var = *stack_->back();
        stack_->pop_back();
        return var.sptr();
    }

    Address pop_address()
    {
        auto res = top_address();
        stack_->pop_back();
        return res;
    }

    Size size() const
    {
        return stack_->size();
    }

    Stack *get_stack()
    {
        return stack_.get();
    }

    std::filesystem::path &current_path()
    {
        return current_path_;
    }

    void set_call_anchor();
    Size get_call_args_num();

    void set_return_anchor();
    Size get_return_vals_num();

  private:
    SPtr<Context> pre_context_;
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size pc_;
    SPtr<Stack> stack_;
    std::filesystem::path current_path_;
};
}
