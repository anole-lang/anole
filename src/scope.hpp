#pragma once

#include <map>
#include <stack>
#include <memory>
#include <string>
#include "code.hpp"

namespace ice_language
{
class Scope
{
  public:
    Scope(std::shared_ptr<Scope> pre) : pre_(pre) {}

    void push(std::shared_ptr<void> value)
    {
        stack_.push(value);
    }
    template <typename R = void>
    std::shared_ptr<R> top()
    {
        return std::reinterpret_pointer_cast<R>(stack_.top());
    }
    template <typename R = void>
    std::shared_ptr<R> pop()
    {
        auto res = top<R>();
        stack_.pop();
        return res;
    }

    std::shared_ptr<void> find_symbol(const std::string &name)
    {
        if (symbols_.count(name))
        {
            return symbols_[name];
        }
        else
        {
            return pre_->find_symbol(name);
        }
    }

    std::shared_ptr<void> load_symbol(const std::string &name)
    {
        auto find = find_symbol(name);
        if (find == nullptr)
        {
            return symbols_[name] = std::make_shared<void>(nullptr);
        }
        else
        {
            return find;
        }
    }

    void execute_ins(Instruction &ins)
    {
        ins.execute(*this);
    }

    void execute_code(Code &code)
    {
        for (auto ins : code.get_instructions())
        {
            ins->execute(*this);
        }
    }

  private:
    std::shared_ptr<Scope> pre_;
    std::stack<std::shared_ptr<void>> stack_;
    std::map<std::string, std::shared_ptr<void>> symbols_;
};
}
