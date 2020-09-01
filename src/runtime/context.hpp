#pragma once

#include "scope.hpp"
#include "allocator.hpp"

#include "../base.hpp"
#include "../error.hpp"
#include "../compiler/instruction.hpp"

#include <any>
#include <map>
#include <list>
#include <stack>
#include <filesystem>

namespace anole
{
class Code;

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
    add_not_defined_symbol(const String &name,
        const Address addr
    );

    static void
    rm_not_defined_symbol(const Address addr);

    static const String
    &get_not_defined_symbol(const Address addr);

  public:
    // this for resume from ContObject
    Context(SPtr<Context> resume);

    // copy ctor
    Context(const Context &context);

    Context(SPtr<Code> code,
        std::filesystem::path path = std::filesystem::current_path()
    );

    Context(SPtr<Context> pre, SPtr<Scope> scope,
        SPtr<Code> code, Size pc = 0
    );

    SPtr<Context> &pre_context();
    SPtr<Scope> &scope();
    SPtr<Code> &code();
    Size &pc();

    const Instruction &ins();
    const Instruction &ins_at(Size index);
    const Opcode opcode();
    const std::any &oprand();

    void push(ObjectSPtr sptr);
    void push(Address addr);

    template<typename R = Object>
    R *top_rptr()
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

    Address top_address()
    {
        return stack_->back();
    }

    void set_top(Address addr)
    {
        stack_->back() = addr;
    }

    void set_top(ObjectSPtr sptr)
    {
        stack_->back() = Allocator<Variable>::alloc(move(sptr));
    }

    void pop()
    {
        stack_->pop_back();
    }

    template<typename R = Object>
    R *pop_rptr()
    {
        auto &var = *stack_->back();
        stack_->pop_back();
        return reinterpret_cast<R *>(var.rptr());
    }

    /**
     * use pop_sptr when binding it with other variables
    */
    ObjectSPtr pop_sptr()
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

    std::stack<Size> call_anchors_;
    Size return_anchor_;
};
}
