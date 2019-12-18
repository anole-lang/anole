#include "code.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"

#define OPRAND(TYPE) std::reinterpret_pointer_cast<TYPE>(ins.oprand)

using namespace std;

namespace ice_language
{
Code::Code()
  : constants_
    {
        TheNoneObject,
        TheTrueBool,
        TheFalseBool
    }
{
    // ...
}

size_t Code::size()
{
    return instructions_.size();
}

vector<Instruction> &Code::get_instructions()
{
    return instructions_;
}

void Code::push_break(size_t ind)
{
    breaks_.push_back(ind);
}

void Code::set_break_to(size_t ind, size_t base)
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

void Code::push_continue(size_t ind)
{
    continues_.push_back(ind);
}

void Code::set_continue_to(std::size_t ind, std::size_t base)
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

bool Code::check()
{
    if (!breaks_.empty() && !continues_.empty())
    {
        // throw
        return false;
    }
    return true;
}

ObjectPtr Code::load_const(size_t ind)
{
    return constants_[ind];
}

void Code::print(ostream &out)
{
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
        case Op::Create:
            out << i << "\tCreate\t\t" << *OPRAND(std::string) << std::endl;
            break;
        case Op::Load:
            out << i << "\tLoad\t\t" << *OPRAND(std::string) << std::endl;
            break;
        case Op::LoadConst:
            out << i << "\tLoadConst\t" << *OPRAND(std::size_t) << std::endl;
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
        case Op::Mul:
            out << i << "\tMul" << std::endl;
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
}
}
