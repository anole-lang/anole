#pragma once

#include <any>
#include <vector>
#include <string>
#include <utility>
#include "helper.hpp"

namespace ice_language
{
enum Opcode
{
    PlaceHolder,

    Pop,          // done

    Create,       // done
    Load,         // done
    LoadConst,    // done
    LoadMember,   // done
    Store,        // done

    Neg,          // done
    Add,          // done
    Sub,          // done
    Mul,          // done
    Div,          // done
    Mod,          // done

    CEQ,          // done
    CNE,          // done
    CLT,          // done
    CLE,          // done

    Index,        // done

    ScopeBegin,   // done
    ScopeEnd,     // done

    Call,         // done
    CallTail,     // done
    Return,       // done
    Jump,         // done
    JumpIf,       // done
    JumpIfNot,    // done
    Match,        // done
    LambdaDecl,   // done
    ThunkDecl,    // done

    BuildList,    // done
    BuildDict,    // done
};

struct Instruction
{
    Opcode opcode;
    std::any oprand;
};
}
