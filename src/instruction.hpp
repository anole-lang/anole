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

    Pop,          // Pop

    Import,       // Import name
    ImportPart,   // ImportPart name
    ImportAll,    // ImportAll name

    Load,         // Load name
    LoadConst,    // LoadConst index
    LoadMember,   // LoadMember name
    Store,        // Store
    StoreLocal,   // StoreLocal name

    ScopeBegin,   // ScopeBegin
    ScopeEnd,     // ScopeEnd

    Call,         // Call num
    CallTail,     // CallTail num
    Return,       // Return
    Jump,         // Jump target
    JumpIf,       // JumpIf target
    JumpIfNot,    // JumpIfNot target
    Match,        // Match target

    LambdaDecl,   // LambdaDecl target
    ThunkDecl,    // ThunkDecl target

    Neg,          // Neg
    Add,          // Add
    Sub,          // Sub
    Mul,          // Mul
    Div,          // Div
    Mod,          // Mod

    Is,           // Is
    CEQ,          // CEQ
    CNE,          // CNE
    CLT,          // CLT
    CLE,          // CLE

    Index,        // Index

    BuildList,    // BuildList num
    BuildDict,    // BuildDict num
};

struct Instruction
{
    Opcode opcode;
    std::any oprand;
};
}
