#include <set>
#include <fstream>
#include "base.hpp"
#include "code.hpp"
#include "version.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "floatobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

#define OPRAND(TYPE) (any_cast<const TYPE &>(ins.oprand))

using namespace std;
using namespace anole;

namespace
{
class Printer
{
  public:
    Printer(ostream &out) : out_(out) {}

    template<typename ...Ts>
    void add_line(Ts ...args)
    {
        lines_.emplace_back();
        if constexpr (sizeof...(Ts) > 0)
        {
            gen_line(lines_.back(), args...);
        }
    }

    void add_intro(String intro)
    {
        lines_.push_back({intro});
    }

    void print()
    {
        for (auto &line : lines_)
        {
            for (Size i = 0; i < line.size(); ++i)
            {
                out_ << line[i];
                if (i < line.size() - 1)
                {
                    out_ << String(lens_[i] - line[i].size(), ' ');
                }
            }
            out_ << endl;
        }
    }

    void clear()
    {
        lines_.clear();
        lens_.clear();
    }

  private:
    String gen_each(Size value)
    {
        return to_string(value);
    }

    String gen_each(const char *value)
    {
        return value;
    }

    String gen_each(String value)
    {
        return value;
    }

    template<typename T1, typename T2>
    String gen_each(pair<T1, T2> value)
    {
        return "(" + gen_each(value.first) + ", " + gen_each(value.second) + ")";
    }

    template<typename T, typename ...Ts>
    void gen_line(vector<String> &line, T arg, Ts ...args)
    {
        auto str = gen_each(arg);
        if (lens_.size() <= line.size())
        {
            lens_.push_back(0);
        }
        if (str.size() > lens_[line.size()])
        {
            lens_[line.size()] = (str.size() / 4 + 1) * 4;
        }
        line.push_back(move(str));
        if constexpr (sizeof...(Ts) > 0)
        {
            gen_line(line, args...);
        }
    }

    ostream &out_;
    vector<vector<String>> lines_;
    vector<Size> lens_;
};

template<typename T>
void typeout(ostream &out, const T &value)
{
    const char *chrs = reinterpret_cast<const char *>(&value);
    for (Size i = 0; i < sizeof(T); ++i)
    {
        out.put(*chrs++);
    }
}

void typeout(ostream &out, const String &value)
{
    typeout(out, value.size());
    for (const auto c : value)
    {
        out.put(c);
    }
}

template<typename T, typename ...Args>
void typeouts(ostream &out, const T &arg, const Args &...args)
{
    typeout(out, arg);
    if constexpr (sizeof...(Args) > 0)
    {
        typeouts(out, args...);
    }
}

// not partial specialization
template<typename T1, typename T2>
void typeout(ostream &out, const pair<T1, T2> &pir)
{
    typeouts(out, pir.first, pir.second);
}

template<typename T>
void typein(istream &in, T &target)
{
    char temp[sizeof(T)];
    char *p = temp;
    for (Size i = 0; i < sizeof(T); ++i)
    {
        *p++ = in.get();
    }
    target = *(reinterpret_cast<T*>(temp));
}

void typein(istream &in, String &value)
{
    Size len;
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

template<typename T1, typename T2>
void typein(istream &in, pair<T1, T2> &pir)
{
    typeins(in, pir.first, pir.second);
}
}

namespace anole
{
Code::Code(String from)
  : from_(move(from))
  , constants_{ theNone, theTrue, theFalse }
{
    // ...
}

Size Code::size()
{
    return instructions_.size();
}

void Code::push_break(Size ind)
{
    breaks_.push_back(ind);
}

void Code::set_break_to(Size ind, Size base)
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

void Code::push_continue(Size ind)
{
    continues_.push_back(ind);
}

void Code::set_continue_to(Size ind, Size base)
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

ObjectPtr Code::load_const(Size ind)
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
    Printer printer{out};

    printer.add_intro("Constants:");
    printer.add_line("CI", "Value");

    for (Size i = 0; i < constants_literals_.size(); ++i)
    {
        printer.add_line(i + 3, constants_literals_[i]);
    }

    printer.add_line();
    printer.print();
    printer.clear();

    printer.add_intro("Instructions:");
    printer.add_line("L", "Opcode", "Oprand");

    for (Size i = 0; i < instructions_.size(); ++i)
    {
        auto &ins = instructions_[i];
        switch (ins.opcode)
        {
        case Opcode::PlaceHolder:
            break;

        case Opcode::Pop:
            printer.add_line(i, "Pop");
            break;

        case Opcode::Import:
            printer.add_line(i, "Import", OPRAND(String));
            break;
        case Opcode::ImportPath:
            printer.add_line(i, "ImportPath", OPRAND(String));
            break;
        case Opcode::ImportAll:
            printer.add_line(i, "ImportAll", OPRAND(String));
            break;
        case Opcode::ImportAllPath:
            printer.add_line(i, "ImportAllPath", OPRAND(String));
            break;
        case Opcode::ImportPart:
            printer.add_line(i, "ImportPart", OPRAND(String));
            break;

        case Opcode::Load:
            printer.add_line(i, "Load", OPRAND(String));
            break;
        case Opcode::LoadConst:
            printer.add_line(i, "LoadConst", OPRAND(Size));
            break;
        case Opcode::LoadMember:
            printer.add_line(i, "LoadMember", OPRAND(String));
            break;
        case Opcode::Store:
            printer.add_line(i, "Store");
            break;
        case Opcode::StoreRef:
            printer.add_line(i, "StoreRef", OPRAND(String));
            break;
        case Opcode::StoreLocal:
            printer.add_line(i, "StoreLocal", OPRAND(String));
            break;

        case Opcode::NewScope:
            printer.add_line(i, "NewScope");
            break;

        case Opcode::CallAc:
            printer.add_line(i, "CallAc");
            break;
        case Opcode::Call:
            printer.add_line(i, "Call");
            break;
        case Opcode::FastCall:
            printer.add_line(i, "FastCall");
            break;
        case Opcode::ReturnAc:
            printer.add_line(i, "ReturnAc");
            break;
        case Opcode::Return:
            printer.add_line(i, "Return");
            break;
        case Opcode::ReturnNone:
            printer.add_line(i, "ReturnNone");
            break;
        case Opcode::Jump:
            printer.add_line(i, "Jump", OPRAND(Size));
            break;
        case Opcode::JumpIf:
            printer.add_line(i, "JumpIf", OPRAND(Size));
            break;
        case Opcode::JumpIfNot:
            printer.add_line(i, "JumpIfNot", OPRAND(Size));
            break;
        case Opcode::Match:
            printer.add_line(i, "Match", OPRAND(Size));
            break;

        case Opcode::AddPrefixOp:
            printer.add_line(i, "AddPrefixOp", OPRAND(String));
            break;

        case Opcode::AddInfixOp:
        {
            using type = pair<String, Size>;
            printer.add_line(i, "AddInfixOp", OPRAND(type));
        }
            break;

        case Opcode::Pack:
            printer.add_line(i, "Pack");
            break;
        case Opcode::Unpack:
            printer.add_line(i, "Unpack");
            break;

        case Opcode::LambdaDecl:
        {
            using type = pair<Size, Size>;
            printer.add_line(i, "LambdaDecl", OPRAND(type));
        }
            break;
        case Opcode::ThunkDecl:
            printer.add_line(i, "ThunkDecl", OPRAND(Size));
            break;
        case Opcode::ThunkOver:
            printer.add_line(i, "ThunkOver");
            break;

        case Opcode::Neg:
            printer.add_line(i, "Neg");
            break;
        case Opcode::Add:
            printer.add_line(i, "Add");
            break;
        case Opcode::Sub:
            printer.add_line(i, "Sub");
            break;
        case Opcode::Mul:
            printer.add_line(i, "Mul");
            break;
        case Opcode::Div:
            printer.add_line(i, "Div");
            break;
        case Opcode::Mod:
            printer.add_line(i, "Mod");
            break;

        case Opcode::Is:
            printer.add_line(i, "Is");
            break;
        case Opcode::CEQ:
            printer.add_line(i, "CEQ");
            break;
        case Opcode::CNE:
            printer.add_line(i, "CNE");
            break;
        case Opcode::CLT:
            printer.add_line(i, "CLT");
            break;
        case Opcode::CLE:
            printer.add_line(i, "CLE");
            break;

        case Opcode::BNeg:
            printer.add_line(i, "BNeg");
            break;
        case Opcode::BOr:
            printer.add_line(i, "BOr");
            break;
        case Opcode::BXor:
            printer.add_line(i, "BXor");
            break;
        case Opcode::BAnd:
            printer.add_line(i, "BAnd");
            break;
        case Opcode::BLS:
            printer.add_line(i, "BLS");
            break;
        case Opcode::BRS:
            printer.add_line(i, "BRS");
            break;

        case Opcode::Index:
            printer.add_line(i, "Index");
            break;

        case Opcode::BuildEnum:
            printer.add_line(i, "BuildEnum");
            break;
        case Opcode::BuildList:
            printer.add_line(i, "BuildList", OPRAND(Size));
            break;
        case Opcode::BuildDict:
            printer.add_line(i, "BuildDict", OPRAND(Size));
            break;
        }
    }
    printer.print();
}

void Code::serialize(const filesystem::path &path)
{
    ofstream fout{path};
    serialize(fout);
}

void Code::serialize(ostream &out)
{
    typeout(out, theMagic);

    typeouts(out, constants_literals_.size(),
                  instructions_.size(),
                  mapping_.size());

    for (Size i = 0; i < constants_literals_.size(); ++i)
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
        }
    }

    for (auto &ins : instructions_)
    {
        out.put(ins.opcode);
        switch (ins.opcode)
        {
        case Opcode::LoadConst:
        case Opcode::Jump:
        case Opcode::JumpIf:
        case Opcode::JumpIfNot:
        case Opcode::Match:
        case Opcode::ThunkDecl:
        case Opcode::BuildList:
        case Opcode::BuildDict:
            typeout(out, OPRAND(Size));
            break;

        case Opcode::Import:
        case Opcode::ImportPath:
        case Opcode::ImportAll:
        case Opcode::ImportAllPath:
        case Opcode::ImportPart:
        case Opcode::Load:
        case Opcode::LoadMember:
        case Opcode::StoreRef:
        case Opcode::StoreLocal:
        case Opcode::AddPrefixOp:
            typeout(out, OPRAND(String));
            break;

        case Opcode::AddInfixOp:
        {
            using type = pair<String, Size>;
            typeout(out, OPRAND(type));
        }
            break;

        case Opcode::LambdaDecl:
        {
            using type = pair<Size, Size>;
            typeout(out, OPRAND(type));
        }
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

bool Code::unserialize(const filesystem::path &path)
{
    ifstream fin{path};
    return unserialize(fin);
}

bool Code::unserialize(ifstream &in)
{
    Magic magic; typein(in, magic);
    if (magic != theMagic)
    {
        return false;
    }

    Size constants_size = 0,
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
            String val;
            typein(in, val);
            constants_literals_.push_back(type + val);
            constants_.push_back(make_shared<StringObject>(val));
        }
            break;

        default:
            throw runtime_error("WTF, you want me to eat shit?!");
        }
    }

    while (instructions_size --> 0)
    {
        Opcode opcode = Opcode(in.get());
        std::any oprand = {};
        switch (opcode)
        {
        case Opcode::LoadConst:
        case Opcode::Jump:
        case Opcode::JumpIf:
        case Opcode::JumpIfNot:
        case Opcode::Match:
        case Opcode::ThunkDecl:
        case Opcode::BuildList:
        case Opcode::BuildDict:
        {
            Size val;
            typein(in, val);
            oprand = val;
        }
            break;

        case Opcode::Import:
        case Opcode::ImportPath:
        case Opcode::ImportAll:
        case Opcode::ImportAllPath:
        case Opcode::ImportPart:
        case Opcode::Load:
        case Opcode::LoadMember:
        case Opcode::StoreRef:
        case Opcode::StoreLocal:
        case Opcode::AddPrefixOp:
        {
            String val;
            typein(in, val);
            oprand = val;
        }
            break;

        case Opcode::AddInfixOp:
        {
            pair<String, Size> val;
            typein(in, val);
            oprand = val;
        }
            break;

        case Opcode::LambdaDecl:
        {
            pair<Size, Size> val;
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
        Size line, pf, ps;
        typeins(in, line, pf, ps);
        mapping_[line] = { pf, ps };
    }

    return true;
}
}
