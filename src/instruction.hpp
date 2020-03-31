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

    Create,       // Create name
    Load,         // Load name
    LoadConst,    // LoadConst index
    LoadMember,   // LoadMember name
    Store,        // Store
    StoreLocal,   // StoreLocal name

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

    ScopeBegin,   // ScopeBegin
    ScopeEnd,     // ScopeEnd

    Call,         // Call
    CallTail,     // CallTail
    Return,       // Return
    Jump,         // Jump target
    JumpIf,       // JumpIf target
    JumpIfNot,    // JumpIfNot target
    Match,        // Match target

    MakeClosure,  // MakeClosure {names...}
    LambdaDecl,   // LambdaDecl target
    ThunkDecl,    // ThunkDecl target

    BuildList,    // BuildList nums
    BuildDict,    // BuildDict nums
};

struct Instruction
{
    Opcode opcode;
    std::any oprand;
};
}
