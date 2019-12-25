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
    code.add_ins<Op::ScopeBegin>();
    for (auto statement : statements)
    {
        statement->codegen(code);
    }
    code.add_ins<Op::ScopeEnd>();
}

// completed
void NoneExpr::codegen(Code &code)
{
    code.add_ins<Op::LoadConst>(0);
}

// completed
void IntegerExpr::codegen(Code &code)
{
    code.add_ins<Op::LoadConst>(
        code.create_const<IntegerObject>(
            'i' + to_string(value), value
        )
    );
}

// completed
void FloatExpr::codegen(Code &code)
{
    code.add_ins<Op::LoadConst>(
        code.create_const<FloatObject>(
            'f' + to_string(value), value
        )
    );
}

// completed
void BoolExpr::codegen(Code &code)
{
    code.add_ins<Op::LoadConst>(
        static_cast<size_t>(value ? 1 : 2)
    );
}

// completed
void StringExpr::codegen(Code &code)
{
    code.add_ins<Op::LoadConst>(
        code.create_const<StringObject>(
            's' + value, value
        )
    );
}

// completed
void IdentifierExpr::codegen(Code &code)
{
    code.add_ins<Op::Load>(name);
}

// completed
void ParenOperatorExpr::codegen(Code &code)
{
    for (auto arg : args)
    {
        arg->codegen(code);
    }
    expr->codegen(code);
    code.add_ins<Op::Call>(args.size());
}

// completed
void UnaryOperatorExpr::codegen(Code &code)
{
    expr->codegen(code);
    switch (op)
    {
    case TokenId::Sub:
        code.add_ins<Op::Neg>();
        break;

    case TokenId::Not:
    {
        auto o1 = code.add_ins();
        code.add_ins<Op::LoadConst>(static_cast<size_t>(1)); // theTrue
        auto o2 = code.add_ins();
        code.set_ins<Op::JumpIf>(o1, code.size());
        code.add_ins<Op::LoadConst>(static_cast<size_t>(2)); // theFalse
        code.set_ins<Op::Jump>(o2, code.size());
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
        code.add_ins<Op::Store>();
        break;

    case TokenId::Add:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::Add>();
        break;

    case TokenId::Sub:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::Sub>();
        break;

    case TokenId::Mul:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::Mul>();
        break;

    case TokenId::Div:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::Div>();
        break;

    case TokenId::Mod:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::Mod>();
        break;

    case TokenId::And:
    {
        lhs->codegen(code);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        auto o2 = code.add_ins();
        code.add_ins<Op::LoadConst>(static_cast<size_t>(1));
        auto o3 = code.add_ins();
        code.set_ins<Op::JumpIfNot>(o1, code.size());
        code.set_ins<Op::JumpIfNot>(o2, code.size());
        code.add_ins<Op::LoadConst>(static_cast<size_t>(2));
        code.set_ins<Op::Jump>(o3, code.size());
    }
        break;

    case TokenId::Or:
    {
        lhs->codegen(code);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        auto o2 = code.add_ins();
        code.add_ins<Op::LoadConst>(static_cast<size_t>(2));
        auto o3 = code.add_ins();
        code.set_ins<Op::JumpIf>(o1, code.size());
        code.set_ins<Op::JumpIf>(o2, code.size());
        code.add_ins<Op::LoadConst>(static_cast<size_t>(1));
        code.set_ins<Op::Jump>(o3, code.size());
    }
        break;

    case TokenId::CEQ:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::CEQ>();
        break;

    case TokenId::CNE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::CNE>();
        break;

    case TokenId::CLT:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::CLT>();
        break;

    case TokenId::CLE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.add_ins<Op::CLE>();
        break;

    case TokenId::CGT:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Op::CLE>();
        break;

    case TokenId::CGE:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Op::CLT>();
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
        code.add_ins<Op::Load>(arg_decl->id->name);
        code.add_ins<Op::Store>();
        code.add_ins<Op::Pop>();
    }
    block->codegen(code);
    code.add_ins<Op::LoadConst>(0);
    code.add_ins<Op::Return>();
    code.set_ins<Op::LambdaDecl>(o1, arg_decls.size());
    code.set_ins<Op::LambdaDecl>(o2, code.size());
}

// [AFTER] [CLASS]
void NewExpr::codegen(Code &code)
{

}

// [AFTER] [CLASS]
void DotExpr::codegen(Code &code)
{

}

// [AFTER] [CLASS]
void EnumExpr::codegen(Code &code)
{

}

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
            code.set_ins<Op::Match>(where, code.size());
        }
        values[i]->codegen(code);
        jumpfroms.push_back(code.add_ins());
    }

    for (auto where : jumpfroms)
    {
        code.set_ins<Op::Jump>(where, code.size());
    }
}

// [AFTER] [CLASS]
void ListExpr::codegen(Code &code)
{

}

// [AFTER] [CLASS]
void IndexExpr::codegen(Code &code)
{

}

// [AFTER] [CLASS]
void DictExpr::codegen(Code &code)
{

}

// completed
void DelayExpr::codegen(Code &code)
{
    auto o1 = code.add_ins();
    expr->codegen(code);
    code.add_ins<Op::Return>();
    code.set_ins<Op::ThunkDecl>(o1, code.size());
}

// completed
void QuesExpr::codegen(Code &code)
{
    cond->codegen(code);
    auto o1 = code.add_ins();
    true_expr->codegen(code);
    auto o2 = code.add_ins();
    code.set_ins<Op::JumpIfNot>(o1, code.size());
    false_expr->codegen(code);
    code.set_ins<Op::Jump>(o2, code.size());
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
        code.add_ins<Op::Pop>();
    }
}

// completed
void VariableDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Op::Create>(id->name);
    if (expr)
    {
        expr->codegen(code);
        code.add_ins<Op::Load>(id->name);
        code.add_ins<Op::Store>();
    }
}

// completed
void FunctionDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Op::Create>(id->name);
    lambda->codegen(code);
    code.add_ins<Op::Load>(id->name);
    code.add_ins<Op::Store>();
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
    code.add_ins<Op::Return>();
}

// completed
void IfElseStmt::codegen(Code &code)
{
    cond->codegen(code);
    auto o1 = code.add_ins();
    block_true->codegen(code);
    if (else_stmt)
    {
        auto o2 = code.add_ins();
        code.set_ins<Op::JumpIfNot>(o1, code.size());
        else_stmt->codegen(code);
        code.set_ins<Op::Jump>(o2, code.size());
    }
    else
    {
        code.set_ins<Op::JumpIfNot>(o1, code.size());
    }
}

// completed
void WhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    cond->codegen(code);
    auto o2 = code.add_ins();
    block->codegen(code);
    code.add_ins<Op::Jump>(o1);
    code.set_ins<Op::JumpIfNot>(o2, code.size());
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o1, o1);
}

// completed
void DoWhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    block->codegen(code);
    auto o2 = code.size();
    cond->codegen(code);
    code.add_ins<Op::JumpIf>(o1);
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o2, o1);
}

void ForStmt::codegen(Code &code)
{

}

void ForeachStmt::codegen(Code &code)
{

}
}
