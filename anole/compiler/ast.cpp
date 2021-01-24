#include "ast.hpp"

namespace anole
{
AST::~AST() = default;
Stmt::~Stmt() = default;
Expr::~Expr() = default;
DeclarationStmt::~DeclarationStmt() = default;

bool AST::is_integer_expr() noexcept
{
    return false;
}

bool IntegerExpr::is_integer_expr() noexcept
{
    return true;
}

bool AST::is_expr_stmt() noexcept
{
    return false;
}

bool ExprStmt::is_expr_stmt() noexcept
{
    return true;
}

StringExpr::StringExpr(String value) noexcept
  : value(std::move(value))
{
    // ...
}

IdentifierExpr::IdentifierExpr(String name) noexcept
  : name(std::move(name))
{
    // ...
}

ParenOperatorExpr::ParenOperatorExpr(Ptr<Expr> &&expr, ArgumentList &&args) noexcept
  : expr(std::move(expr)), args(std::move(args))
{
    // ...
}

UnaryOperatorExpr::UnaryOperatorExpr(Token op, Ptr<Expr> &&expr) noexcept
  : op(op), expr(std::move(expr))
{
    // ...
}

BinaryOperatorExpr::BinaryOperatorExpr(Ptr<Expr> &&lhs, Token op, Ptr<Expr> &&rhs) noexcept
  : op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
{
    // ...
}

LambdaExpr::LambdaExpr(ParameterList &&parameters, Ptr<BlockExpr> &&block) noexcept
  : parameters(std::move(parameters)), block(std::move(block))
{
    // ...
}

DotExpr::DotExpr(Ptr<Expr> &&left, String name) noexcept
  : left(std::move(left)), name(std::move(name))
{
    // ...
}

IndexExpr::IndexExpr(Ptr<Expr> &&expr, Ptr<Expr> &&index) noexcept
  : expr(std::move(expr)), index(std::move(index))
{
    // ...
}

ClassExpr::ClassExpr(String name, ArgumentList &&bases, DeclList &&members) noexcept
  : name(std::move(name)), bases(std::move(bases)), members(std::move(members))
{
    // ...
}

DelayExpr::DelayExpr(Ptr<Expr> &&expr) noexcept
  : expr(std::move(expr))
{
    // ...
}

QuesExpr::QuesExpr(Ptr<Expr> &&cond, Ptr<Expr> &&true_expr, Ptr<Expr> &&false_expr) noexcept
  : cond(std::move(cond)), true_expr(std::move(true_expr)), false_expr(std::move(false_expr))
{
    // ...
}

UseStmt::UseStmt(Aliases &&aliases, UseStmt::NestedModule &&from) noexcept
  : aliases(std::move(aliases)), from(std::move(from))
{
    // ...
}

ExprStmt::ExprStmt(Ptr<Expr> &&expr) noexcept
  : expr(std::move(expr))
{
    // ...
}

VariableDeclarationStmt::VariableDeclarationStmt(String name, Ptr<Expr> &&expr, bool is_ref) noexcept
  : name(std::move(name)), expr(std::move(expr)), is_ref(is_ref)
{
    // ...
}

MultiVarsDeclarationStmt::MultiVarsDeclarationStmt(std::list<VariableDeclarationStmt> &&decls, ExprList &&exprs) noexcept
  : decls(std::move(decls)), exprs(std::move(exprs))
{
    // ...
}

PrefixopDeclarationStmt::PrefixopDeclarationStmt(String op) noexcept
  : op(std::move(op))
{
    // ...
}

InfixopDeclarationStmt::InfixopDeclarationStmt(String op, Size precedence) noexcept
  : precedence(precedence), op(std::move(op))
{
    // ...
}

ReturnStmt::ReturnStmt(ExprList &&exprs) noexcept
  : exprs(std::move(exprs))
{
    // ...
}

IfElseStmt::IfElseStmt(Ptr<Expr> &&cond, Ptr<BlockExpr> &&true_block, Ptr<AST> &&false_branch) noexcept
  : cond(std::move(cond)), true_block(std::move(true_block)), false_branch(std::move(false_branch))
{
    // ...
}

WhileStmt::WhileStmt(Ptr<Expr> &&cond, Ptr<BlockExpr> &&block) noexcept
  : cond(std::move(cond)), block(std::move(block))
{
    // ...
}

DoWhileStmt::DoWhileStmt(Ptr<Expr> &&cond, Ptr<BlockExpr> &&block) noexcept
  : cond(std::move(cond)), block(std::move(block))
{
    // ...
}

ForeachStmt::ForeachStmt(Ptr<Expr> &&expr, String varname, Ptr<BlockExpr> &&block) noexcept
  : expr(std::move(expr)), varname(std::move(varname)), block(std::move(block))
{
    // ...
}
} // namespace anole
