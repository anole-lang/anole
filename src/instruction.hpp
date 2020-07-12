#pragma once

#include <any>

namespace anole
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
    StoreRef,     // StoreRef name
    StoreLocal,   // StoreLocal name

    NewScope,     // NewScope

    Call,         // Call n
    CallTail,     // CallTail n
    CallExAnchor, // CallExAnchor
    CallEx,       // CallEx
    CallExTail,   // CallExTail
    Return,       // Return n
    Jump,         // Jump target
    JumpIf,       // JumpIf target
    JumpIfNot,    // JumpIfNot target
    Match,        // Match target

    AddPrefixOp,  // AddPrefixOp op
    AddInfixOp,   // AddInfixOp (op, priority)

    Pack,         // Pack
    Unpack,       // Unpack

    LambdaDecl,   // LambdaDecl (num, target)
    ThunkDecl,    // ThunkDecl target
    ThunkOver,    // ThunkOver

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

    BNeg,         // BNeg
    BOr,          // BOr
    BXor,         // BXor
    BAnd,         // BAnd
    BLS,          // BLS
    BRS,          // BRS

    Index,        // Index

    BuildEnum,    // BuildEnum
    BuildList,    // BuildList num
    BuildDict,    // BuildDict num
};

struct Instruction
{
    Opcode opcode;
    std::any oprand;
};
}
