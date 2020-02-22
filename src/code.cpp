#include "code.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

#define OPRAND(TYPE) (any_cast<TYPE>(ins.oprand))

using namespace std;

namespace ice_language
{
Code::Code()
  : constants_
    {
        theNone,
        theTrue,
        theFalse
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
            set_ins<Opcode::Jump>(i, ind);
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
            set_ins<Opcode::Jump>(i, ind);
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
    out << "Constants:\nConsIndex\tValue" << endl;

    for (size_t i = 0; i < constants_literals_.size(); ++i)
    {
        out << i + 3 << "\t\t" << constants_literals_[i] << endl;
    }

    out << "\nInstructions:\nLine\tOp" << endl;

    for (size_t i = 0; i < instructions_.size(); ++i)
    {
        auto &ins = instructions_[i];
        switch (ins.opcode)
        {
        case Opcode::Pop:
            out << i << "\tPop" << std::endl;
            break;
        case Opcode::Create:
            out << i << "\tCreate\t\t" << OPRAND(string) << std::endl;
            break;
        case Opcode::Load:
            out << i << "\tLoad\t\t" << OPRAND(string) << std::endl;
            break;
        case Opcode::LoadConst:
            out << i << "\tLoadConst\t" << OPRAND(std::size_t) << std::endl;
            break;
        case Opcode::LoadMember:
            out << i << "\tLoadMember\t" << OPRAND(std::string) << std::endl;
            break;
        case Opcode::Store:
            out << i << "\tStore" << std::endl;
            break;

        case Opcode::Neg:
            out << i << "\tNeg" << std::endl;
            break;
        case Opcode::Add:
            out << i << "\tAdd" << std::endl;
            break;
        case Opcode::Sub:
            out << i << "\tSub" << std::endl;
            break;
        case Opcode::Mul:
            out << i << "\tMul" << std::endl;
            break;
        case Opcode::Div:
            out << i << "\tDiv" << std::endl;
            break;
        case Opcode::Mod:
            out << i << "\tMod" << std::endl;
            break;

        case Opcode::CEQ:
            out << i << "\tCEQ" << std::endl;
            break;
        case Opcode::CNE:
            out << i << "\tCNE" << std::endl;
            break;
        case Opcode::CLT:
            out << i << "\tCLT" << std::endl;
            break;
        case Opcode::CLE:
            out << i << "\tCLE" << std::endl;
            break;

        case Opcode::Index:
            out << i << "\tIndex" << std::endl;
            break;

        case Opcode::ScopeBegin:
            out << i << "\tScopeBegin" << std::endl;
            break;
        case Opcode::ScopeEnd:
            out << i << "\tScopeEnd" << std::endl;
            break;

        case Opcode::Call:
            out << i << "\tCall\t\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::CallTail:
            out << i << "\tCallTail\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::Return:
            out << i << "\tReturn" << std::endl;
            break;
        case Opcode::Jump:
            out << i << "\tJump\t\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::JumpIf:
            out << i << "\tJumpIf\t\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::JumpIfNot:
            out << i << "\tJumpIfNot\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::Match:
            out << i << "\tMatch\t\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::LambdaDecl:
            out << i << "\tLambdaDecl\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::ThunkDecl:
            out << i << "\tThunkDecl\t" << OPRAND(size_t) << std::endl;
            break;

        case Opcode::BuildList:
            out << i << "\tBuildList\t" << OPRAND(size_t) << std::endl;
            break;
        case Opcode::BuildDict:
            out << i << "\tBuildDict\t" << OPRAND(size_t) << std::endl;
            break;

        default:
            break;
        }
    }
}

void Code::to_file(ostream &out)
{
    // ...
}
}
