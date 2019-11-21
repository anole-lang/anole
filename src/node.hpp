#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include "token.hpp"

#define INTERPRET_DECL

namespace ice_language
{
class Code;

template <typename T>
using Ptr         = std::shared_ptr<T>;
using StmtList    = std::vector<Ptr<struct Stmt>>;
using ExprList    = std::vector<Ptr<struct Expr>>;
using IdentList   = std::vector<Ptr<struct IdentifierExpr>>;
using VarDeclList = std::vector<Ptr<struct VariableDeclarationStmt>>;

struct Node
{
    virtual ~Node() = default;
    virtual void codegen(Code &) = 0;
};

struct Stmt : Node {};
struct Expr : Node {};

struct BlockExpr : Expr
{
    StmtList statements;
    INTERPRET_DECL;
};

struct NoneExpr : Expr
{
    INTERPRET_DECL;
};

struct IntegerExpr : Expr
{
    long value;
    IntegerExpr(long value) : value(value) {}
    INTERPRET_DECL;
};

struct FloatExpr : Expr
{
    double value;
    FloatExpr(double value) : value(value) {}
    INTERPRET_DECL;
};

struct BoolExpr : Expr
{
    bool value;
    BoolExpr(bool value) : value(value) {}
    INTERPRET_DECL;
};

struct StringExpr : Expr
{
    std::string value;
    StringExpr(std::string value) : value(std::move(value)) {}
    INTERPRET_DECL;
};

struct IdentifierExpr : Expr
{
    std::string name;
    IdentifierExpr(std::string name) : name(std::move(name)) {}
    INTERPRET_DECL;
};

struct ParenOperatorExpr : Expr
{
    Ptr<Expr> expr;
    ExprList args;
    ParenOperatorExpr(Ptr<Expr> expr, ExprList args) : expr(expr), args(args) {}
    INTERPRET_DECL;
};

struct UnaryOperatorExpr : Expr
{
    TOKEN op;
    Ptr<Expr> expr;
    UnaryOperatorExpr(TOKEN op, Ptr<Expr> expr) : op(op), expr(expr) {}
    INTERPRET_DECL;
};

struct BinaryOperatorExpr : Expr
{
    TOKEN op;
    Ptr<Expr> lhs, rhs;
    BinaryOperatorExpr(Ptr<Expr> lhs, TOKEN op, Ptr<Expr> rhs) : op(op), lhs(lhs), rhs(rhs) {}
    INTERPRET_DECL;
};

struct LambdaExpr : Expr
{
    VarDeclList argDecls;
    Ptr<BlockExpr> block;
    LambdaExpr(VarDeclList argDecls, Ptr<BlockExpr> block) : argDecls(argDecls), block(block) {}
    INTERPRET_DECL;
};

struct NewExpr : Expr
{
    Ptr<IdentifierExpr> id;
    ExprList args;
    NewExpr(Ptr<IdentifierExpr> id, ExprList args) : id(id), args(args) {}
    INTERPRET_DECL;
};

struct DotExpr : Expr
{
    Ptr<Expr> left, right;
    DotExpr(Ptr<Expr> left, Ptr<Expr> right) : left(left), right(right) {}
    INTERPRET_DECL;
};

struct EnumExpr : Expr
{
    IdentList idents;
    EnumExpr(IdentList idents) : idents(idents) {}
    INTERPRET_DECL;
};

struct MatchExpr : Expr
{
    Ptr<Expr> expr;
    ExprList match_exprs, return_exprs;
    Ptr<Expr> else_expr;
    MatchExpr(Ptr<Expr> expr, ExprList match_exprs, ExprList return_exprs, Ptr<Expr> else_expr)
      : expr(expr), match_exprs(match_exprs), return_exprs(return_exprs), else_expr(else_expr) {}
    INTERPRET_DECL;
};

struct ListExpr : Expr
{
    ExprList exprs;
    ListExpr(ExprList exprs) : exprs(exprs) {}
    INTERPRET_DECL;
};

struct IndexExpr : Expr
{
    Ptr<Expr> expr, index;
    IndexExpr(Ptr<Expr> expr, Ptr<Expr> index) : expr(expr), index(index) {}
    INTERPRET_DECL;
};

struct DictExpr : Expr
{
    ExprList keys, values;
    DictExpr() {}
    DictExpr(ExprList keys, ExprList values) : keys(keys), values(values) {}
    INTERPRET_DECL;
};

struct UsingStmt : Stmt
{
    std::string name;
    UsingStmt(std::string name) : name(std::move(name)) {}
    INTERPRET_DECL;
};

struct ExprStmt : Stmt
{
    Ptr<Expr> expr;
    ExprStmt(Ptr<Expr> expr) : expr(expr) {}
    INTERPRET_DECL;
};

struct VariableDeclarationStmt : Stmt
{
    Ptr<Expr> left, expr;
    VariableDeclarationStmt(Ptr<Expr> left, Ptr<Expr> expr)
      : left(left), expr(expr) {}
    INTERPRET_DECL;
};

struct VariableAssignStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    Ptr<Expr> expr;
    VariableAssignStmt(Ptr<IdentifierExpr> id, Ptr<Expr>expr) : id(id), expr(expr) {}
    INTERPRET_DECL;
};

struct FunctionDeclarationStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    VarDeclList argDecls;
    Ptr<BlockExpr> block;
    FunctionDeclarationStmt(Ptr<IdentifierExpr> id, VarDeclList argDecls, Ptr<BlockExpr> block)
      : id(id), argDecls(argDecls), block(block) {}
    INTERPRET_DECL;
};

struct ClassDeclarationStmt : Stmt
{
    Ptr<IdentifierExpr> id;
    IdentList bases;
    Ptr<BlockExpr> block;
    ClassDeclarationStmt(Ptr<IdentifierExpr> id, IdentList bases, Ptr<BlockExpr> block)
      : id(id), bases(bases), block(block) {}
    INTERPRET_DECL;
};

struct BreakStmt : Stmt
{
    INTERPRET_DECL;
};

struct ContinueStmt : Stmt
{
    INTERPRET_DECL;
};

struct ReturnStmt : Stmt
{
    Ptr<Expr> expr;
    ReturnStmt(Ptr<Expr> expr) :expr(expr) {}
    INTERPRET_DECL;
};

struct IfElseStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> blockTrue;
    Ptr<IfElseStmt> elseStmt;
    IfElseStmt(Ptr<Expr> cond, Ptr<BlockExpr> blockTrue, Ptr<IfElseStmt> elseStmt)
      : cond(cond), blockTrue(blockTrue), elseStmt(elseStmt) {}
    INTERPRET_DECL;
};

struct WhileStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;
    WhileStmt(Ptr<Expr> cond, Ptr<BlockExpr> block) : cond(cond), block(block) {}
    INTERPRET_DECL;
};

struct DoWhileStmt : Stmt
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;
    DoWhileStmt(Ptr<Expr> cond, Ptr<BlockExpr> block) : cond(cond), block(block) {}
    INTERPRET_DECL;
};

struct ForStmt : Stmt
{
    Ptr<Expr> begin, end;
    Ptr<IdentifierExpr> id;
    Ptr<BlockExpr> block;
    ForStmt(Ptr<Expr> begin, Ptr<Expr> end, Ptr<IdentifierExpr> id, Ptr<BlockExpr> block)
      : begin(begin), end(end), id(id), block(block) {}
    INTERPRET_DECL;
};

struct ForeachStmt : Stmt
{
    Ptr<Expr> expr;
    Ptr<IdentifierExpr> id;
    Ptr<BlockExpr> block;
    ForeachStmt(Ptr<Expr> expr, Ptr<IdentifierExpr> id, Ptr<BlockExpr> block)
      : expr(expr), id(id), block(block) {}
    INTERPRET_DECL;
};
}

#undef INTERPRET_DECL
