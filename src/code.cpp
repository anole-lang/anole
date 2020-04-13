#include <set>
#include <fstream>
#include "code.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "floatobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

#define OPRAND(TYPE) (any_cast<TYPE>(ins.oprand))

using namespace std;

namespace
{
template<typename T>
void typeout(ostream &out, T value)
{
    char *chrs = reinterpret_cast<char *>(&value);
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        out.put(*chrs++);
    }
}

template<>
void typeout<string>(ostream &out, string value)
{
    typeout(out, value.size());
    for (auto c : value)
    {
        out.put(c);
    }
}

template<typename T, typename ...Args>
void typeouts(ostream &out, T arg, Args... args)
{
    typeout(out, arg);
    if constexpr (sizeof...(Args) > 0)
    {
        typeouts(out, args...);
    }
}

template<typename T>
void typein(istream &in, T &target)
{
    char temp[sizeof(T)];
    char *p = temp;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        *p++ = in.get();
    }
    target = *(reinterpret_cast<T*>(temp));
}

template<>
void typein(istream &in, string &value)
{
    size_t len;
    typein(in, len);
    while (len--)
    {
        value += in.get();
    }
}

template<typename T, typename ...Args>
void typeins(istream &in, T &arg, Args &...args)
{
    typein(in, arg);
    if constexpr (sizeof...(Args) > 0)
    {
        typeins(in, args...);
    }
}
}

namespace ice_language
{
Code::Code(string from)
  : from_(move(from))
  , constants_{ theNone, theTrue, theFalse }
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

size_t &Code::nested_scopes()
{
    static size_t value = 0;
    return value;
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

void Code::set_continue_to(size_t ind, size_t base)
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

void Code::print(const filesystem::path &path)
{
    ofstream fout{path};
    print(fout);
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
        case Opcode::PlaceHolder:
            break;

        case Opcode::Pop:
            out << i << "\tPop" << endl;
            break;

        case Opcode::Import:
            out << i << "\tImport\t\t" << OPRAND(string) << endl;
            break;
        case Opcode::ImportPart:
            out << i << "\tImportPart\t" << OPRAND(string) << endl;
            break;
        case Opcode::ImportAll:
            out << i << "\tImportAll\t" << OPRAND(string) << endl;
            break;

        case Opcode::Load:
            out << i << "\tLoad\t\t" << OPRAND(string) << endl;
            break;
        case Opcode::LoadConst:
            out << i << "\tLoadConst\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::LoadMember:
            out << i << "\tLoadMember\t" << OPRAND(string) << endl;
            break;
        case Opcode::Store:
            out << i << "\tStore" << endl;
            break;
        case Opcode::StoreLocal:
            out << i << "\tStoreLocal\t" << OPRAND(string) << endl;
            break;

        case Opcode::ScopeBegin:
            out << i << "\tScopeBegin" << endl;
            break;
        case Opcode::ScopeEnd:
            out << i << "\tScopeEnd" << endl;
            break;

        case Opcode::Call:
            out << i << "\tCall\t\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::CallTail:
            out << i << "\tCallTail\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::Return:
            out << i << "\tReturn" << endl;
            break;
        case Opcode::Jump:
            out << i << "\tJump\t\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::JumpIf:
            out << i << "\tJumpIf\t\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::JumpIfNot:
            out << i << "\tJumpIfNot\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::Match:
            out << i << "\tMatch\t\t" << OPRAND(size_t) << endl;
            break;

        case Opcode::LambdaDecl:
            out << i << "\tLambdaDecl\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::ThunkDecl:
            out << i << "\tThunkDecl\t" << OPRAND(size_t) << endl;
            break;

        case Opcode::Neg:
            out << i << "\tNeg" << endl;
            break;
        case Opcode::Add:
            out << i << "\tAdd" << endl;
            break;
        case Opcode::Sub:
            out << i << "\tSub" << endl;
            break;
        case Opcode::Mul:
            out << i << "\tMul" << endl;
            break;
        case Opcode::Div:
            out << i << "\tDiv" << endl;
            break;
        case Opcode::Mod:
            out << i << "\tMod" << endl;
            break;

        case Opcode::Is:
            out << i << "\tIs" << endl;
            break;
        case Opcode::CEQ:
            out << i << "\tCEQ" << endl;
            break;
        case Opcode::CNE:
            out << i << "\tCNE" << endl;
            break;
        case Opcode::CLT:
            out << i << "\tCLT" << endl;
            break;
        case Opcode::CLE:
            out << i << "\tCLE" << endl;
            break;

        case Opcode::BNeg:
            out << i << "\tBNeg" << endl;
            break;
        case Opcode::BOr:
            out << i << "\tBOr" << endl;
            break;
        case Opcode::BXor:
            out << i << "\tBXor" << endl;
            break;
        case Opcode::BAnd:
            out << i << "\tBAnd" << endl;
            break;
        case Opcode::BLS:
            out << i << "\tBLS" << endl;
            break;
        case Opcode::BRS:
            out << i << "\tBRS" << endl;
            break;

        case Opcode::Index:
            out << i << "\tIndex" << endl;
            break;

        case Opcode::BuildEnum:
            out << i << "\tBuildEnum" << endl;
            break;
        case Opcode::BuildList:
            out << i << "\tBuildList\t" << OPRAND(size_t) << endl;
            break;
        case Opcode::BuildDict:
            out << i << "\tBuildDict\t" << OPRAND(size_t) << endl;
            break;
        }
    }
}

void Code::serialize(const filesystem::path &path)
{
    ofstream fout{path};
    serialize(fout);
}

// out should be opened by binary mode
void Code::serialize(ostream &out)
{
    typeouts(out, constants_literals_.size(),
                  instructions_.size(),
                  mapping_.size());

    for (size_t i = 0; i < constants_literals_.size(); ++i)
    {
        auto &cl = constants_literals_[i];
        auto &obj = constants_[i + 3];

        out.put(cl[0]);
        switch (cl[0])
        {
        case 'i':
            typeout(out, reinterpret_pointer_cast<IntegerObject>(obj)->value());
            break;
        case 'f':
            typeout(out, reinterpret_pointer_cast<FloatObject>(obj)->value());
            break;
        case 's':
            typeout(out, cl.substr(1));
            break;

        default:
            throw runtime_error("WTF, here is a bug!");
            break;
        }
    }

    for (auto &ins : instructions_)
    {
        out.put(ins.opcode);
        switch (ins.opcode)
        {
        case Opcode::LoadConst:
        case Opcode::Call:
        case Opcode::CallTail:
        case Opcode::Jump:
        case Opcode::JumpIf:
        case Opcode::JumpIfNot:
        case Opcode::Match:
        case Opcode::LambdaDecl:
        case Opcode::ThunkDecl:
        case Opcode::BuildList:
        case Opcode::BuildDict:
            typeout(out, OPRAND(size_t));
            break;

        case Opcode::Import:
        case Opcode::ImportPart:
        case Opcode::ImportAll:
        case Opcode::Load:
        case Opcode::LoadMember:
        case Opcode::StoreLocal:
            typeout(out, OPRAND(string));
            break;

        default:
            break;
        }
    }

    for (auto &line_pos : mapping_)
    {
        typeouts(out, line_pos.first,
            line_pos.second.first, line_pos.second.second);
    }
}

void Code::unserialize(const filesystem::path &path)
{
    ifstream fin{path};
    unserialize(fin);
}

// in should be opened by binary mode
void Code::unserialize(ifstream &in)
{
    size_t constants_size = 0,
           instructions_size = 0,
           mapping_size = 0;
    typeins(in, constants_size, instructions_size, mapping_size);

    while (constants_size --> 0)
    {
        char type = in.get();
        switch (type)
        {
        case 'i':
        {
            int64_t val;
            typein(in, val);
            constants_literals_.push_back(type + to_string(val));
            constants_.push_back(make_shared<IntegerObject>(val));
        }
            break;

        case 'f':
        {
            double val;
            typein(in, val);
            constants_literals_.push_back(type + to_string(val));
            constants_.push_back(make_shared<FloatObject>(val));
        }
            break;

        case 's':
        {
            string val;
            typein(in, val);
            constants_literals_.push_back(type + val);
            constants_.push_back(make_shared<StringObject>(val));
        }
            break;

        default:
            throw runtime_error("WTF, you want me to eat shit?!");
            break;
        }
    }

    while (instructions_size --> 0)
    {
        Opcode opcode = static_cast<Opcode>(in.get());
        std::any oprand = {};
        switch (opcode)
        {
        case Opcode::LoadConst:
        case Opcode::Call:
        case Opcode::CallTail:
        case Opcode::Jump:
        case Opcode::JumpIf:
        case Opcode::JumpIfNot:
        case Opcode::Match:
        case Opcode::LambdaDecl:
        case Opcode::ThunkDecl:
        case Opcode::BuildList:
        case Opcode::BuildDict:
        {
            size_t val;
            typein(in, val);
            oprand = val;
        }
            break;

        case Opcode::Import:
        case Opcode::ImportPart:
        case Opcode::ImportAll:
        case Opcode::Load:
        case Opcode::LoadMember:
        case Opcode::StoreLocal:
        {
            string val;
            typein(in, val);
            oprand = val;
        }
            break;

        default:
            break;
        }
        instructions_.push_back(Instruction{ opcode, oprand });
    }

    while (mapping_size --> 0)
    {
        size_t line, pf, ps;
        typeins(in, line, pf, ps);
        mapping_[line] = { pf, ps };
    }

}
}
