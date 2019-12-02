#include <tuple>
#include <memory>
#include "ast.hpp"
#include "code.hpp"

using namespace std;

namespace ice_language
{
AST::~AST() = default;
Stmt::~Stmt() = default;
Expr::~Expr() = default;

void BlockExpr::codegen(Code &code)
{
    for (auto statement : statements)
    {
        statement->codegen(code);
    }
}

void NoneExpr::codegen(Code &code)
{
    code.add_ins<Push>(nullptr);
}

void IntegerExpr::codegen(Code &code)
{
    code.add_ins<Push>(value);
}

void FloatExpr::codegen(Code &code)
{
    code.add_ins<Push>(value);
}

void BoolExpr::codegen(Code &code)
{
    code.add_ins<Push>(value);
}

void StringExpr::codegen(Code &code)
{
    code.add_ins<Push>(value);
}

void IdentifierExpr::codegen(Code &code)
{
    code.add_ins<Load>(name);
}

void ParenOperatorExpr::codegen(Code &code)
{
    for (auto arg : args)
    {
        arg->codegen(code);
    }
    expr->codegen(code);
    code.add_ins<Call>(args.size());
}

void UnaryOperatorExpr::codegen(Code &code)
{
    switch (op)
    {
    case TokenId::Sub:
        code.add_ins<Neg>();
        break;

    default:
        break;
    }
}

void BinaryOperatorExpr::codegen(Code &code)
{
    switch (op)
    {
    case TokenId::Add:
        code.add_ins<Add>();
        break;

    case TokenId::Sub:
        code.add_ins<Sub>();
        break;

    default:
        break;
    }
}

void LambdaExpr::codegen(Code &code)
{

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

void UsingStmt::codegen(Code &code)
{

}

void ExprStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Pop>();
}

void VariableDeclarationStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Create>(id->name);
    code.add_ins<Load>(id->name);
    code.add_ins<Store>();
}

void VariableAssignStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Load>(id->name);
    code.add_ins<Store>();
}

void NonVariableAssignStmt::codegen(Code &code)
{
    expr->codegen(code);
    left->codegen(code);
    code.add_ins<Store>();
}

void FunctionDeclarationStmt::codegen(Code &code)
{
    // code will be gen in code
    // but there should be a function object after
    // the code's running
}

void ClassDeclarationStmt::codegen(Code &code)
{

}

void BreakStmt::codegen(Code &code)
{

}

void ContinueStmt::codegen(Code &code)
{

}

void ReturnStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Return>();
}

void IfElseStmt::codegen(Code &code)
{
    cond->codegen(code);
    // code.add_ins<JumpIf>()
}

void WhileStmt::codegen(Code &code)
{

}

void DoWhileStmt::codegen(Code &code)
{

}

void ForStmt::codegen(Code &code)
{

}

void ForeachStmt::codegen(Code &code)
{

}
}
