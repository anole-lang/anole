#ifndef __ANOLE_COMPILER_AST_HPP__
#define __ANOLE_COMPILER_AST_HPP__

#include "token.hpp"

#include <map>
#include <list>
#include <vector>
#include <utility>
#include <optional>

namespace anole
{
class Code;

using ExprList
    = std::list<Ptr<struct Expr>>
;

using ArgumentList
    = std::list<std::pair<Ptr<struct Expr>, bool>>
; // boolean stands for whether it is unpacked

/**
 * as an attribute
 *  because not every node needs locations
*/
struct with_location
{
    Location location;
};
struct with_locations
{
    std::vector<Location> locations;
};

struct AST
{
    virtual ~AST() = 0;
    virtual bool is_integer_expr() noexcept;
    virtual bool is_expr_stmt() noexcept;
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
    using StmtList
        = std::list<Ptr<Stmt>>
    ;

    StmtList statements;

    void codegen(Code &) override;
};

struct NoneExpr : Expr
{
    void codegen(Code &) override;
};

struct IntegerExpr : Expr
{
    int64_t value;

    IntegerExpr(int64_t value) noexcept
      : value(value)
    {
        // ...
    }

    bool is_integer_expr() noexcept override;
    void codegen(Code &) override;
};

struct FloatExpr : Expr
{
    double value;

    FloatExpr(double value) noexcept
      : value(value)
    {
        // ...
    }

    void codegen(Code &) override;
};

struct BoolExpr : Expr
{
    bool value;

    BoolExpr(bool value) noexcept
      : value(value)
    {
        // ...
    }

    void codegen(Code &) override;
};

struct StringExpr : Expr
{
    String value;

    StringExpr(String) noexcept;
    void codegen(Code &) override;
};

struct IdentifierExpr : Expr, with_location
{
    String name;

    IdentifierExpr(String) noexcept;
    void codegen(Code &) override;
};

struct ParenOperatorExpr : Expr, with_location
{
    Ptr<Expr> expr;
    ArgumentList args;

    ParenOperatorExpr(Ptr<Expr> &&, ArgumentList &&) noexcept;
    void codegen(Code &) override;
};

struct UnaryOperatorExpr : Expr, with_location
{
    Token op;
    Ptr<Expr> expr;

    UnaryOperatorExpr(Token, Ptr<Expr> &&) noexcept;
    void codegen(Code &) override;
};

struct BinaryOperatorExpr : Expr, with_location
{
    Token op;
    Ptr<Expr> lhs, rhs;

    BinaryOperatorExpr(Ptr<Expr> &&, Token, Ptr<Expr> &&) noexcept;
    void codegen(Code &) override;
};

struct LambdaExpr : Expr
{
    using ParameterList
        = std::list<std::pair<Ptr<struct NormalDeclarationStmt>, bool>>
    ; // boolean stands for whether it is packed

    ParameterList parameters;
    Ptr<BlockExpr> block;

    LambdaExpr(Ptr<BlockExpr> &&) noexcept;
    LambdaExpr(ParameterList &&, Ptr<BlockExpr> &&) noexcept;
    void codegen(Code &) override;
};

struct DotExpr : Expr, with_location
{
    Ptr<Expr> left;
    String name;

    DotExpr(Ptr<Expr> &&, String) noexcept;
    void codegen(Code &) override;
};

struct EnumExpr : Expr
{
    std::list<struct NormalDeclarationStmt> decls;

    void codegen(Code &) override;
};

struct MatchExpr : Expr, with_locations
{
    Ptr<Expr> expr;
    std::vector<ExprList> keylists;
    std::vector<Ptr<Expr>> values;
    Ptr<Expr> else_expr;

    void codegen(Code &) override;
};

struct ListExpr : Expr, with_location
{
    ExprList exprs;

    void codegen(Code &) override;
};

struct IndexExpr : Expr, with_location
{
    Ptr<Expr> expr, index;

    IndexExpr(Ptr<Expr> &&, Ptr<Expr> &&) noexcept;
    void codegen(Code &) override;
};

struct DictExpr : Expr
{
    ExprList keys, values;

    void codegen(Code &) override;
};

struct ClassExpr : Expr
{
    using DeclList
        = std::list<Ptr<struct DeclarationStmt>>
    ;

    String name;
    ArgumentList bases;
    DeclList members;

    ClassExpr(String, ArgumentList &&, DeclList &&) noexcept;
    void codegen(Code &) override;
};

struct DelayExpr : Expr
{
    Ptr<Expr> expr;

    DelayExpr(Ptr<Expr> &&) noexcept;
    void codegen(Code &) override;
};

struct QuesExpr : Expr, with_location
{
    Ptr<Expr> cond, true_expr, false_expr;

    QuesExpr(Ptr<Expr> &&, Ptr<Expr> &&, Ptr<Expr> &&) noexcept;
    void codegen(Code &) override;
};

struct UseStmt : Stmt
{
    struct Module
    {
        enum class Type
        {
            Name,
            Path
        };

        String mod;
        Type type;
    };

    using NestedModule
        = std::vector<Module>
    ;

    using Alias
        = std::pair<NestedModule, String>
    ; // second String for the alias

    using Aliases
        = std::list<Alias>
    ;

    // empty alias means `use *`
    Aliases aliases;
    // may be empty
    NestedModule from;

    UseStmt(Aliases &&, NestedModule &&) noexcept;
    void codegen(Code &) override;
};

struct ExprStmt : Stmt
{
    Ptr<Expr> expr;

    ExprStmt(Ptr<Expr> &&) noexcept;
    bool is_expr_stmt() noexcept override;
    void codegen(Code &) override;
};

struct DeclarationStmt : Stmt
{

    virtual ~DeclarationStmt() = 0;
    virtual void codegen(Code &) = 0;
};

struct NormalDeclarationStmt : DeclarationStmt
{
    bool is_ref;
    String name;
    Ptr<Expr> expr;

    NormalDeclarationStmt(String, Ptr<Expr> &&, bool = false) noexcept;
    void codegen(Code &) override;
};

/**
 * correct:
 *  @a, b: 1, 2;     // positional
 *  @a, b: [1, 2];   // unpack
 *  @[a, b]: [1, 2]; // unpack
 *
 * wrong:
 *  @[a, b]: 1, 2;
*/
struct MultiVarsDeclarationStmt : DeclarationStmt
{
    struct DeclVariable
    {
        virtual ~DeclVariable() = 0;
        virtual void codegen(Code &) = 0;
    };
    using DeclVariableList
        = std::list<Ptr<DeclVariable>>
    ;

    struct SingleDeclVariable : DeclVariable
    {
        bool is_ref;
        String name;

        SingleDeclVariable(String, bool = false) noexcept;
        void codegen(Code &) override;
    };

    struct MultiDeclVariables : DeclVariable
    {
        DeclVariableList variables;

        MultiDeclVariables(DeclVariableList &&) noexcept;
        void codegen(Code &) override;
    };

    DeclVariableList variables;
    ExprList exprs;

    MultiVarsDeclarationStmt(DeclVariableList &&variables) noexcept;
    MultiVarsDeclarationStmt(DeclVariableList &&variables, ExprList &&exprs) noexcept;
    void codegen(Code &) override;
};

struct PrefixopDeclarationStmt : Stmt
{
    String op;

    PrefixopDeclarationStmt(String) noexcept;
    void codegen(Code &) override;
};

struct InfixopDeclarationStmt : Stmt
{
    Size precedence;
    String op;

    InfixopDeclarationStmt(String, Size) noexcept;
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
    ExprList exprs;

    ReturnStmt() noexcept;
    ReturnStmt(ExprList &&) noexcept;
    void codegen(Code &) override;
};

struct IfElseStmt : Stmt, with_location
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> true_block;
    Ptr<AST> false_branch;

    IfElseStmt(Ptr<Expr> &&, Ptr<BlockExpr> &&, Ptr<AST> &&) noexcept;
    void codegen(Code &) override;
};

struct WhileStmt : Stmt, with_location
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;

    WhileStmt(Ptr<Expr> &&, Ptr<BlockExpr> &&) noexcept;
    void codegen(Code &) override;
};

struct DoWhileStmt : Stmt, with_location
{
    Ptr<Expr> cond;
    Ptr<BlockExpr> block;

    DoWhileStmt(Ptr<Expr> &&, Ptr<BlockExpr> &&) noexcept;
    void codegen(Code &) override;
};

struct ForeachStmt : Stmt
{
    Ptr<Expr> expr;
    String varname;
    Ptr<BlockExpr> block;

    ForeachStmt(Ptr<Expr> &&, String, Ptr<BlockExpr> &&) noexcept;
    void codegen(Code &) override;
};
}

#endif
