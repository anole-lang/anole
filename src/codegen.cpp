#include <tuple>
#include <memory>
#include "ast.hpp"
#include "code.hpp"

using namespace std;

namespace ice_language
{
void BlockExpr::codegen(Code &code)
{

}

void NoneExpr::codegen(Code &code)
{

}

void IntegerExpr::codegen(Code &code)
{
    code.add_ins({
        Opcode::Push,
        { make_shared<long>(value) }
    });
}

void FloatExpr::codegen(Code &code)
{
    code.add_ins({
        Opcode::Push,
        { make_shared<double>(value) }
    });
}

void BoolExpr::codegen(Code &code)
{

}

void StringExpr::codegen(Code &code)
{

}

void IdentifierExpr::codegen(Code &code)
{

}

void ParenOperatorExpr::codegen(Code &code)
{

}

void UnaryOperatorExpr::codegen(Code &code)
{

}

void BinaryOperatorExpr::codegen(Code &code)
{
    switch (op)
    {
    case TokenId::Add:
        code.add_ins({
            Opcode::Add
        });
        break;

    default:
        break;
    }
}

void LambdaExpr::codegen(Code &code)
{

}

void NewExpr::codegen(Code &code)
{

}

void DotExpr::codegen(Code &code)
{

}

void EnumExpr::codegen(Code &code)
{

}

void MatchExpr::codegen(Code &code)
{

}

void ListExpr::codegen(Code &code)
{

}

void IndexExpr::codegen(Code &code)
{

}

void DictExpr::codegen(Code &code)
{

}

void UsingStmt::codegen(Code &code)
{

}

void ExprStmt::codegen(Code &code)
{

}

void VariableDeclarationStmt::codegen(Code &code)
{

}

void VariableAssignStmt::codegen(Code &code)
{

}

void FunctionDeclarationStmt::codegen(Code &code)
{

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

}

void IfElseStmt::codegen(Code &code)
{

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
