#pragma once

#include <vector>
#include <iostream>
#include <type_traits>
#include "helper.hpp"
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    Code() = default;

    template <Op op = Op::PlaceHolder>
    std::size_t add_ins()
    {
        instructions_.push_back({
            op, nullptr
        });
        return instructions_.size() - 1;
    }

    template <Op op, typename T>
    std::size_t add_ins(T value)
    {
        if constexpr (op == Op::Push
            and std::is_same<T, std::nullptr_t>::value)
        {
            instructions_.push_back({
                Op::Push, nullptr
            });
        }
        else
        {
            instructions_.push_back({
                op, std::make_shared<T>(value)
            });
        }
        return instructions_.size() - 1;
    }

    template <Op op>
    void set_ins(std::size_t ind)
    {
        instructions_[ind] = {op, nullptr};
    }

    template <Op op, typename T>
    void set_ins(std::size_t ind, T value)
    {
        if constexpr (op == Op::Push
            and std::is_same<T, std::nullptr_t>::value)
        {
            instructions_[ind] = {
                Op::Push, nullptr
            };
        }
        else
        {
            instructions_[ind] = {
                op, std::make_shared<T>(value)
            };
        }
    }

    std::size_t size()
    {
        return instructions_.size();
    }

    std::vector<Instruction> &get_instructions()
    {
        return instructions_;
    }

    void push_break(std::size_t ind)
    {
        breaks_.push_back(ind);
    }

    void set_break_to(std::size_t ind,
        std::size_t base)
    {
        decltype(breaks_) breaks;
        for (auto i : breaks_)
        {
            if (i > base)
            {
                set_ins<Op::Jump>(i, ind);
            }
            else
            {
                breaks.push_back(i);
            }
        }
        breaks_ = breaks;
    }

    void push_continue(std::size_t ind)
    {
        continues_.push_back(ind);
    }

    void set_continue_to(std::size_t ind,
        std::size_t base)
    {
        decltype(continues_) continues;
        for (auto i : continues_)
        {
            if (i > base)
            {
                set_ins<Op::Jump>(i, ind);
            }
            else
            {
                continues.push_back(i);
            }
        }
        continues_ = continues;
    }

    bool check()
    {
        if (!breaks_.empty() && !continues_.empty())
        {
            // throw
            return false;
        }
        return true;
    }

    // Simple Print
    void print(std::ostream &out = std::cout)
    {
#define OPRAND(TYPE) std::reinterpret_pointer_cast<TYPE>(ins.oprand)
        for (std::size_t i = 0; i < instructions_.size(); ++i)
        {
            auto &ins = instructions_[i];
            switch (ins.op)
            {
            case Op::PlaceHolder:
                break;
            case Op::Pop:
                out << i << "\tPop" << std::endl;
                break;
            case Op::Push:
                out << i << "\tPush\t\t" << *OPRAND(long) << std::endl;
                break;
            case Op::Create:
                out << i << "\tCreate\t" << *OPRAND(std::string) << std::endl;
                break;
            case Op::Load:
                out << i << "\tLoad\t\t" << *OPRAND(std::string) << std::endl;
                break;
            case Op::Store:
                out << i << "\tStore" << std::endl;
                break;

            case Op::Neg:
                out << i << "\tNeg" << std::endl;
                break;
            case Op::Add:
                out << i << "\tAdd" << std::endl;
                break;
            case Op::Sub:
                out << i << "\tSub" << std::endl;
                break;

            case Op::ScopeBegin:
                out << i << "\tScopeBegin" << std::endl;
                break;
            case Op::ScopeEnd:
                out << i << "\tScopeEnd" << std::endl;
                break;
            case Op::Call:
                out << i << "\tCall" << std::endl;
                break;
            case Op::Return:
                out << i << "\tReturn" << std::endl;
                break;
            case Op::Jump:
                out << i << "\tJump\t\t" << *OPRAND(size_t) << std::endl;
                break;
            case Op::JumpIf:
                out << i << "\tJumpIf\t\t" << *OPRAND(size_t) << std::endl;
                break;
            case Op::JumpIfNot:
                out << i << "\tJumpIfNot\t" << *OPRAND(size_t) << std::endl;
                break;
            case Op::LambdaDecl:
                out << i << "\tLambdaDecl\t" << *OPRAND(size_t) << std::endl;
                break;
            case Op::ThunkDecl:
                out << i << "\tThunkDecl\t" << *OPRAND(size_t) << std::endl;
                break;
            }
        }
#undef OPRAND
    }

  private:
    std::vector<Instruction> instructions_;
    // these two should be checked is empty or not
    std::vector<std::size_t> breaks_, continues_;
};
}
