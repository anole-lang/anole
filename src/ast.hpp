#pragma once

#include <vector>
#include <string>
#include <utility>
#include "helper.hpp"
#include "token.hpp"

namespace ice_language
{
class Code;

using StmtList     = std::vector<Ptr<struct Stmt>>;
using ExprList     = std::vector<Ptr<struct Expr>>;
using IdentList    = std::vector<Ptr<struct IdentifierExpr>>;
using VarDeclList  = std::vector<Ptr<struct VariableDeclarationStmt>>;

struct AST
{
    virtual ~AST() = 0;
    virtual void codegen(Code &) = 0;
};

struct Stmt : AST
{
    virtual ~Stmt() = 0;
    virtual void codegen(Code &) = 0;
};

struct Expr : AST
{
    virtual ~Expr() = 0;
    virtual void codegen(Code &) = 0;
};

struct BlockExpr : Expr
{
    StmtList statements;
    void codegen(Code &) override;
};

struct NoneExpr : Expr
{
    void codegen(Code &) override;
};

struct IntegerExpr : Expr
{
    long value;
    IntegerExpr(long value) : value(value) {}
    void codegen(Code &) override;
};

struct FloatExpr : Expr
{
    double value;
    FloatExpr(double value) : value(value) {}
    void codegen(Code &) override;
};

struct BoolExpr : Expr
{
    bool value;
    BoolExpr(bool value) : value(value) {}
    void codegen(Code &) override;
};

struct StringExpr : Expr
{
    std::string value;
    StringExpr(std::string value)
      : value(std::move(value)) {}
    void codegen(Code &) override;
};

struct IdentifierExpr : Expr
{
    std::string name;
    IdentifierExpr(std::string name)
      : name(std::move(name)) {}
    void codegen(Code &) override;
};

struct ParenOperatorExpr : Expr
{
    Ptr<Expr> expr;
    ExprList args;
    ParenOperatorExpr(Ptr<Expr> expr, ExprList args)
      : expr(expr), args(args) {}
    void codegen(Code &) override;
};

struct UnaryOperatorExpr : Expr
{
    TokenId op;
    Ptr<Expr> expr;
    UnaryOperatorExpr(TokenId op, Ptr<Expr> expr)
      : op(op), expr(expr) {}
    void codegen(Code &) override;
};

struct BinaryOperatorExpr : Expr
{
    TokenId op;
    Ptr<Expr> lhs, rhs;
    BinaryOperatorExpr(Ptr<Expr> lhs,
        TokenId op, Ptr<Expr> rhs)
      : op(op), lhs(lhs), rhs(rhs) {}
    void codegen(Code &) override;
};

struct LambdaExpr : Expr
{
    VarDeclList arg_decls;
    Ptr<BlockExpr> block;
    LambdaExpr(VarDeclList arg_decls,
        Ptr<BlockExpr> block)
      : arg_decls(arg_decls), block(block) {}
    void codegen(Code &) override;
};

struct NewExpr : Expr
{
    Ptr<IdentifierExpr> id;
    ExprList args;
    NewExpr(Ptr<IdentifierExpr> id, ExprList args)
      : id(id), args(args) {}
    void codegen(Code &) override;
};

struct DotExpr : Expr
{
    Ptr<Expr> left, right;
    DotExpr(Ptr<Expr> left, Ptr<Expr> right)
      : left(left), right(right) {}
    void codegen(Code &) override;
};

struct EnumExpr : Expr
{
    IdentList idents;
    EnumExpr(IdentList idents) : idents(idents) {}
    void codegen(Code &) override;
};

struct MatchExpr : Expr
{
    Ptr<Expr> expr;
    ExprList match_exprs, return_exprs;
    Ptr<Expr> else_expr;
    MatchExpr(Ptr<Expr> expr, ExprList match_exprs,
        ExprList return_exprs, Ptr<Expr> else_expr)
      : expr(expr), match_exprs(match_exprs),
        return_exprs(return_exprs), else_expr(else_expr) {}
    void codegen(Code &) override;
};

struct ListExpr : Expr
{
    ExprList exprs;
    ListExpr(ExprList exprs) : exprs(exprs) {}
    void codegen(Code &) override;
};

struct IndexExpr : Expr
{
    Ptr<Expr> expr, index;
    IndexExpr(Ptr<Expr> expr, Ptr<Expr> index)
      : expr(expr), index(index) {}
    void codegen(Code &) override;
};

struct DictExpr : Expr
{
    ExprList keys, values;
    DictExpr() {}
    DictExpr(ExprList keys, ExprList values)
      : keys(keys), values(values) {}
    void codegen(Code &) override;
};

struct DelayExpr : Expr
{
    Ptr<Expr> expr;
    DelayExpr(Ptr<Expr> expr) : expr(expr) {}
    void codegen(Code &) override;
};

struct UsingStmt : Stmt
{
    std::string name;
    UsingStmt(std::string name)
      : name(std::move(name)) {}
    void codegen(Code &) override;
};

struct ExprStmt : Stmt
{
    Ptr<Expr> expr;
    ExprStmt(Ptr<Expr> expr) : expr(expr) {}
    void codegen(Code &) override;
};

struct VariableDeclarationStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    Ptr<Expr> expr;
    VariableDeclarationStmt(Ptr<IdentifierExpr> id,
        Ptr<Expr>expr) : id(id), expr(expr) {}
    void codegen(Code &) override;
};

struct FunctionDeclarationStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    Ptr<LambdaExpr> lambda;
    FunctionDeclarationStmt(Ptr<IdentifierExpr> id,
        Ptr<LambdaExpr> lambda)
      : id(id), lambda(lambda) {}
    void codegen(Code &) override;
};

struct ClassDeclarationStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    IdentList bases;
    Ptr<BlockExpr> block;
    ClassDeclarationStmt(Ptr<IdentifierExpr> id,
        IdentList bases, Ptr<BlockExpr> block)
      : id(id), bases(bases), block(block) {}
    void codegen(Code &) override;
};

struct BreakStmt : Stmt
{
    void codegen(Code &) override;
};

struct ContinueStmt : Stmt
{
    void codegen(Code &) override;
};

struct ReturnStmt : Stmt
{
    Ptr<Expr> expr;
    ReturnStmt(Ptr<Expr> expr) :expr(expr) {}
    void codegen(Code &) override;
};

struct IfElseStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block_true;
    Ptr<IfElseStmt> else_stmt;
    IfElseStmt(Ptr<Expr> cond, Ptr<BlockExpr> block_true,
        Ptr<IfElseStmt> else_stmt)
      : cond(cond), block_true(block_true),
        else_stmt(else_stmt) {}
    void codegen(Code &) override;
};

struct WhileStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;
    WhileStmt(Ptr<Expr> cond, Ptr<BlockExpr> block)
      : cond(cond), block(block) {}
    void codegen(Code &) override;
};

struct DoWhileStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;
    DoWhileStmt(Ptr<Expr> cond, Ptr<BlockExpr> block)
      : cond(cond), block(block) {}
    void codegen(Code &) override;
};

struct ForStmt : Stmt
{
    Ptr<Expr> begin, end;
    Ptr<IdentifierExpr> id;
    Ptr<BlockExpr> block;
    ForStmt(Ptr<Expr> begin, Ptr<Expr> end,
        Ptr<IdentifierExpr> id, Ptr<BlockExpr> block)
      : begin(begin), end(end), id(id), block(block) {}
    void codegen(Code &) override;
};

struct ForeachStmt : Stmt
{
    Ptr<Expr> expr;
    Ptr<IdentifierExpr> id;
    Ptr<BlockExpr> block;
    ForeachStmt(Ptr<Expr> expr, Ptr<IdentifierExpr> id,
        Ptr<BlockExpr> block)
      : expr(expr), id(id), block(block) {}
    void codegen(Code &) override;
};
}
