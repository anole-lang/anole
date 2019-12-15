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
    Push,         // done

    Create,       // done
    Load,         // done
    Store,        // done

    Neg,          // done
    Add,          // done
    Sub,          // done

    ScopeBegin,   // done
    ScopeEnd,     // done
    Call,         // draft
    Return,       // done
    Jump,         // done
    JumpIf,       // draft
    JumpIfNot,    // draft
    LambdaDecl,   // done
    ThunkDecl,
};

struct Instruction
{
    Op op;
    std::shared_ptr<void> oprand;
};
}
