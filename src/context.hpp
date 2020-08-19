#pragma once

#include "base.hpp"
#include "code.hpp"
#include "error.hpp"
#include "scope.hpp"
#include "allocator.hpp"

#include <map>
#include <stack>
#include <filesystem>

namespace anole
{
// Context should be contructed by make_shared
class Context
{
  public:
    using Stack = std::stack<Address>;

  public:
    static Context *&current();

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
    Context(Context *resume)
      : pre_context_(resume->pre_context_)
      , scope_(Allocator<Scope>::alloc(resume->scope_))
      , code_(resume->code_), pc_(resume->pc_)
      , stack_(Allocator<Stack>::alloc(*resume->stack_))
      , current_path_(resume->current_path_)
    {
        // ...
    }

    // copy ctor
    Context(const Context &context)
      : pre_context_(context.pre_context_)
      , scope_(context.scope_)
      , code_(context.code_), pc_(context.pc_)
      , stack_(Allocator<Stack>::alloc(*context.stack_))
      , current_path_(context.current_path_)
    {
        // ...
    }

    Context(SPtr<Code> code,
        std::filesystem::path path = std::filesystem::current_path())
      : pre_context_(nullptr)
      , scope_(Allocator<Scope>::alloc(nullptr))
      , code_(code), pc_(0)
      , stack_(Allocator<Stack>::alloc())
      , current_path_(std::move(path))
    {
        // ...
    }

    Context(Context *pre, Scope *scope,
        SPtr<Code> code, Size pc = 0)
      : pre_context_(pre)
      , scope_(Allocator<Scope>::alloc(scope))
      , code_(std::move(code)), pc_(pc)
      , stack_(pre->stack_)
      , current_path_(pre->current_path_)
    {
        // ...
    }

    Context *&pre_context()
    {
        return pre_context_;
    }

    Scope *&scope()
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

    void push(Object *obj)
    {
        stack_->push(Allocator<Variable>::alloc(obj));
    }

    void push_address(Address addr)
    {
        stack_->push(addr);
    }

    template<typename R = Object>
    R *top()
    {
        if (stack_->top()->obj() == nullptr)
        {
            throw RuntimeError(
                "var named " +
                get_not_defined_symbol(stack_->top()) +
                " doesn't reference to any object"
            );
        }
        return reinterpret_cast<R *>((*stack_->top()).obj());
    }

    Address &top_address()
    {
        return stack_->top();
    }

    void set_top(Object *obj)
    {
        stack_->top() = Allocator<Variable>::alloc(obj);
    }

    Object *pop()
    {
        auto &var = *stack_->top();
        stack_->pop();
        return var.obj();
    }

    Address pop_address()
    {
        auto res = top_address();
        stack_->pop();
        return res;
    }

    Size size() const
    {
        return stack_->size();
    }

    Stack *get_stack()
    {
        return stack_;
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
    Context *pre_context_;
    Scope *scope_;
    SPtr<Code> code_;
    Size pc_;
    Stack *stack_;
    std::filesystem::path current_path_;
};
}
