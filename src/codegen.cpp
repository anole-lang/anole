#include <tuple>
#include <memory>
#include "ast.hpp"
#include "code.hpp"
#include "floatobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

using namespace std;

namespace ice_language
{
AST::~AST() = default;
Stmt::~Stmt() = default;
Expr::~Expr() = default;

// completed
void BlockExpr::codegen(Code &code)
{
    for (auto statement : statements)
    {
        statement->codegen(code);
    }
}

// completed
void NoneExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, size_t>(0);
}

// completed
void IntegerExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<IntegerObject>(
            'i' + to_string(value), value
        )
    );
}

// completed
void FloatExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<FloatObject>(
            'f' + to_string(value), value
        )
    );
}

// completed
void BoolExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, size_t>(value ? 1 : 2);
}

// completed
void StringExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<StringObject>(
            's' + value, value
        )
    );
}

// completed
void IdentifierExpr::codegen(Code &code)
{
    code.add_ins<Opcode::Load>(name);
}

// completed
void ParenOperatorExpr::codegen(Code &code)
{
    for (auto arg : args)
    {
        arg->codegen(code);
    }
    expr->codegen(code);
    code.add_ins<Opcode::Call>(args.size());
}

// completed
void UnaryOperatorExpr::codegen(Code &code)
{
    expr->codegen(code);
    switch (op)
    {
    case TokenId::Sub:
        code.add_ins<Opcode::Neg>();
        break;

    case TokenId::Not:
    {
        auto o1 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(1); // theTrue
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIf>(o1, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(2); // theFalse
        code.set_ins<Opcode::Jump>(o2, code.size());
    }
        break;

    default:
        break;
    }
}

void BinaryOperatorExpr::codegen(Code &code)
{
    switch (op)
    {
    case TokenId::Colon:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::Store>();
        break;

    case TokenId::Add:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::Add>();
        break;

    case TokenId::Sub:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::Sub>();
        break;

    case TokenId::Mul:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::Mul>();
        break;

    case TokenId::Div:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::Div>();
        break;

    case TokenId::Mod:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::Mod>();
        break;

    case TokenId::And:
    {
        lhs->codegen(code);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(1);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
        code.set_ins<Opcode::JumpIfNot>(o2, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(2);
        code.set_ins<Opcode::Jump>(o3, code.size());
    }
        break;

    case TokenId::Or:
    {
        lhs->codegen(code);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(2);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIf>(o1, code.size());
        code.set_ins<Opcode::JumpIf>(o2, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(1);
        code.set_ins<Opcode::Jump>(o3, code.size());
    }
        break;

    case TokenId::CEQ:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::CEQ>();
        break;

    case TokenId::CNE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::CNE>();
        break;

    case TokenId::CLT:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::CLT>();
        break;

    case TokenId::CLE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenId::CGT:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenId::CGE:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::CLT>();
        break;

    default:
        break;
    }
}

// not support default argumnets now
void LambdaExpr::codegen(Code &code)
{
    auto o1 = code.add_ins();
    auto o2 = code.add_ins();
    for (auto arg_decl : arg_decls)
    {
        code.add_ins<Opcode::Create>(arg_decl->id->name);
        code.add_ins<Opcode::Load>(arg_decl->id->name);
        code.add_ins<Opcode::Store>();
        code.add_ins<Opcode::Pop>();
    }
    block->codegen(code);
    code.add_ins<Opcode::LoadConst, size_t>(0);
    code.add_ins<Opcode::Return>();
    code.set_ins<Opcode::LambdaDecl>(o1, arg_decls.size());
    code.set_ins<Opcode::LambdaDecl>(o2, code.size());
}

// [AFTER] [CLASS]
void NewExpr::codegen(Code &code)
{

}

// completed
void DotExpr::codegen(Code &code)
{
    left->codegen(code);
    code.add_ins<Opcode::LoadMember>(id->name);
}

// [AFTER] [CLASS]
void EnumExpr::codegen(Code &code)
{

}

// completed
void MatchExpr::codegen(Code &code)
{
    expr->codegen(code);

    map<size_t, vector<size_t>> matchfroms;
    vector<size_t> jumpfroms;

    for (size_t i = 0; i < keylists.size(); ++i)
    {
        for (auto &key : keylists[i])
        {
            key->codegen(code);
            matchfroms[i].push_back(code.add_ins());
        }
    }

    else_expr ? else_expr->codegen(code) : NoneExpr().codegen(code);
    jumpfroms.push_back(code.add_ins());

    for (size_t i = 0; i < values.size(); ++i)
    {
        for (auto where : matchfroms[i])
        {
            code.set_ins<Opcode::Match>(where, code.size());
        }
        values[i]->codegen(code);
        jumpfroms.push_back(code.add_ins());
    }

    for (auto where : jumpfroms)
    {
        code.set_ins<Opcode::Jump>(where, code.size());
    }
}

// completed
void ListExpr::codegen(Code &code)
{
    for (auto it = exprs.rbegin();
        it != exprs.rend(); ++it)
    {
        (*it)->codegen(code);
    }
    code.add_ins<Opcode::BuildList>(exprs.size());
}

// completed
void IndexExpr::codegen(Code &code)
{
    index->codegen(code);
    expr->codegen(code);
    code.add_ins<Opcode::Index>();
}

// completed
void DictExpr::codegen(Code &code)
{
    auto num = keys.size();
    while (num--)
    {
        values[num]->codegen(code);
        keys[num]->codegen(code);
    }
    code.add_ins<Opcode::BuildDict>(keys.size());
}

// completed
void DelayExpr::codegen(Code &code)
{
    auto o1 = code.add_ins();
    expr->codegen(code);
    code.add_ins<Opcode::Return>();
    code.set_ins<Opcode::ThunkDecl>(o1, code.size());
}

// completed
void QuesExpr::codegen(Code &code)
{
    cond->codegen(code);
    auto o1 = code.add_ins();
    true_expr->codegen(code);
    auto o2 = code.add_ins();
    code.set_ins<Opcode::JumpIfNot>(o1, code.size());
    false_expr->codegen(code);
    code.set_ins<Opcode::Jump>(o2, code.size());
}

void UsingStmt::codegen(Code &code)
{

}

// completed
void ExprStmt::codegen(Code &code)
{
    expr->codegen(code);
    if (!interpretive())
    {
        code.add_ins<Opcode::Pop>();
    }
}

// completed
void VariableDeclarationStmt::codegen(Code &code)
{
    if (expr)
    {
        expr->codegen(code);
        code.add_ins<Opcode::Create>(id->name);
        id->codegen(code);
        code.add_ins<Opcode::Store>();
    }
    else
    {
        code.add_ins<Opcode::Create>(id->name);
    }
}

// completed
void FunctionDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Opcode::Create>(id->name);
    lambda->codegen(code);
    id->codegen(code);
    code.add_ins<Opcode::Store>();
}

void ClassDeclarationStmt::codegen(Code &code)
{

}

// completed
void BreakStmt::codegen(Code &code)
{
    code.push_break(code.add_ins());
}

// completed
void ContinueStmt::codegen(Code &code)
{
    code.push_continue(code.add_ins());
}

// completed
void ReturnStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Opcode::Return>();
}

// completed
void IfElseStmt::codegen(Code &code)
{
    code.add_ins<Opcode::ScopeBegin>();
    cond->codegen(code);
    auto o1 = code.add_ins();
    block_true->codegen(code);
    if (else_stmt)
    {
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
        else_stmt->codegen(code);
        code.set_ins<Opcode::Jump>(o2, code.size());
    }
    else
    {
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
    }
    code.add_ins<Opcode::ScopeEnd>();
}

// completed
void WhileStmt::codegen(Code &code)
{
    code.add_ins<Opcode::ScopeBegin>();
    auto o1 = code.size();
    cond->codegen(code);
    auto o2 = code.add_ins();
    block->codegen(code);
    code.add_ins<Opcode::Jump>(o1);
    code.set_ins<Opcode::JumpIfNot>(o2, code.size());
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o1, o1);
    code.add_ins<Opcode::ScopeEnd>();
}

// completed
void DoWhileStmt::codegen(Code &code)
{
    code.add_ins<Opcode::ScopeBegin>();
    auto o1 = code.size();
    block->codegen(code);
    auto o2 = code.size();
    cond->codegen(code);
    code.add_ins<Opcode::JumpIf>(o1);
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o2, o1);
    code.add_ins<Opcode::ScopeEnd>();
}

void ForStmt::codegen(Code &code)
{

}

void ForeachStmt::codegen(Code &code)
{

}
}
