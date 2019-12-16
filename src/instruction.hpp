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
    Push,

    Create,       // done
    Load,         // done
    Store,        // done

    Neg,          // done
    Add,          // done
    Sub,          // done

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
