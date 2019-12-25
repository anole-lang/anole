#pragma once

#include <vector>
#include <string>
#include <utility>
#include "helper.hpp"

namespace ice_language
{
enum class Op
{
    PlaceHolder,

    Pop,          // done

    Create,       // done
    Load,         // done
    LoadConst,    // done
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

    ScopeBegin,   // done
    ScopeEnd,     // done

    Call,         // done
    Return,       // done
    Jump,         // done
    JumpIf,       // done
    JumpIfNot,    // done
    Match,        // done
    LambdaDecl,   // done
    ThunkDecl,    // done
};

struct Instruction
{
    Op op;
    std::shared_ptr<void> oprand;
};
}
