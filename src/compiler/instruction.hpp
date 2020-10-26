#ifndef __ANOLE_INSTRUCTION_HPP__
#define __ANOLE_INSTRUCTION_HPP__

#include <any>

namespace anole
{
namespace opcode
{
enum Opcode : uint8_t
{
    PlaceHolder,

    Pop,          // Pop

    Import,       // Import name
    ImportPath,   // ImportPath path
    ImportAll,    // ImportAll name
    ImportAllPath,// ImportAllPath path
    ImportPart,   // ImportPart name

    Load,         // Load name
    LoadConst,    // LoadConst index
    LoadMember,   // LoadMember name
    Store,        // Store
    StoreRef,     // StoreRef name
    StoreLocal,   // StoreLocal name

    NewScope,     // NewScope
    EndScope,     // EndScope

    CallAc,       // CallAc
    Call,         // Call
    FastCall,     // FastCall
    ReturnAc,     // ReturnAc
    Return,       // Return
    ReturnNone,   // ReturnNone
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
    BuildClass,   // BuildClass name
};
}
using opcode::Opcode;

struct Instruction
{
    Opcode opcode;
    std::any oprand;
};
}

#endif
