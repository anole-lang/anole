#ifndef __ANOLE_RUNTIME_CONTEXT_HPP__
#define __ANOLE_RUNTIME_CONTEXT_HPP__

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
class Context;

// special for code in REPL mode
extern SPtr<std::filesystem::path> theWorkingPath;
extern SPtr<Context> theCurrContext;

// Context should be contructed by make_shared
class Context
{
    friend class Collector;

  public:
    using Stack = std::list<Address>;

  public:
    static void set_args(int argc, char *argv[], int start);
    static const std::vector<char *> &get_args();

    static void execute();

  public:
    // this for resume from ContObject
    Context(SPtr<Context> resume);
    // copy ctor
    Context(const Context &context);
    // generally the first ctor
    Context(SPtr<Code> code);
    // ctor for callable objects
    Context(SPtr<Context> pre, SPtr<Scope> scope, SPtr<Code> code, Size pc = 0);

    SPtr<Context> &pre_context();
    SPtr<Scope> &scope();
    SPtr<Code> &code();
    Size &pc() noexcept;

    const Opcode &opcode() const;
    const std::any &oprand() const;

    void push(Object *ptr);
    void push(Address addr);

    template<typename R = Object>
    R *top_ptr()
    {
        if (stack_->back()->ptr() == nullptr)
        {
            throw RuntimeError(
                "var named " +
                stack_->back()->called_name() +
                " doesn't reference to any object"
            );
        }
        return reinterpret_cast<R *>(stack_->back()->ptr());
    }
    const Address &top_address() const;
    void set_top(Address addr);
    void set_top(Object *ptr);
    void pop(Size num = 1);
    template<typename R = Object>
    R *pop_ptr()
    {
        auto ptr = top_ptr<R>();
        stack_->pop_back();
        return reinterpret_cast<R *>(ptr);
    }
    Address pop_address();

    Size size() const;
    Stack *get_stack();
    const std::filesystem::path &code_path() const;

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

    std::stack<Size> call_anchors_;
    Size return_anchor_;
};
}

#endif
