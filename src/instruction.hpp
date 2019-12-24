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
    Div,
    Mod,

    CEQ,
    CNE,
    CLT,
    CLE,
    CGT,
    CGE,

    ScopeBegin,   // done
    ScopeEnd,     // done
    Call,         // done
    Return,       // done
    Jump,         // done
    JumpIf,       // done
    JumpIfNot,    // done
    LambdaDecl,   // done
    ThunkDecl,    // done
};

struct Instruction
{
    Op op;
    std::shared_ptr<void> oprand;
};
}
