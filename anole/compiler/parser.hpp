#ifndef __ANOLE_COMPILER_PARSER_HPP__
#define __ANOLE_COMPILER_PARSER_HPP__

#include "ast.hpp"
#include "tokenizer.hpp"

#include "../error.hpp"

#include <functional>

namespace anole
{
class Parser
{
  public:
    // add new prefix operator
    static void add_prefixop(const String &str);
    // add new infix operator with precedence
    static void add_infixop(const String &str, Size precedence);

  public:
    Parser(std::istream &input, String name_of_input) noexcept;

    void resume();
    void reset();
    void set_resume_action(std::function<void()>);

    Ptr<Stmt> gen_statement();

  private:
    // special name to be like CompileError
    CompileError ParseError(const String &err_info);

    template<TokenType type, TokenType ...types>
    void check(const String &err_info)
    {
        try_resume();

        if (current_token_.type != type)
        {
            if constexpr (sizeof...(types))
            {
                check<types...>(err_info);
            }
            else
            {
                throw ParseError(err_info);
            }
        }
    }
    template<TokenType ...types>
    void eat(const String &err_info = "")
    {
        check<types...>(err_info);
        get_next_token();
    }

    void get_next_token();
    Token &next_token();
    void try_resume(Size times = 0);

    auto gen_exprs()     -> ExprList;
    auto gen_arguments() -> ArgumentList;

    Ptr<Block> gen_block();

    Ptr<Stmt> gen_stmt();
    auto gen_decl_stmt() -> Ptr<DeclarationStmt>;
        auto gen_variable()  -> Ptr<MultiVarsDeclarationStmt::DeclVariable>;
        auto gen_variables() -> MultiVarsDeclarationStmt::DeclVariableList;
    Ptr<Stmt> gen_class_decl_stmt();
    Ptr<Stmt> gen_use_stmt();
        auto gen_module()        -> UseStmt::Module;
        auto gen_nested_module() -> UseStmt::NestedModule;
        auto gen_alias()         -> UseStmt::Alias;
    Ptr<Stmt> gen_prefixop_decl_stmt();
    Ptr<Stmt> gen_infixop_decl_stmt();
    Ptr<Stmt> gen_if_else_stmt();
    Ptr<Stmt> gen_while_stmt();
    Ptr<Stmt> gen_do_while_stmt();
    Ptr<Stmt> gen_foreach_stmt();
    Ptr<Stmt> gen_return_stmt();

    Ptr<Expr> gen_delay_expr();
    Ptr<Expr> gen_expr(int layer = -1);
    Ptr<Expr> gen_term();
    Ptr<Expr> gen_term_tail(Ptr<Expr> expr);
    Ptr<Expr> gen_ident_expr();
        auto gen_ident_rawstr() -> String;
    Ptr<Expr> gen_numeric_expr();
    Ptr<Expr> gen_none_expr();
    Ptr<Expr> gen_boolean_expr();
    Ptr<Expr> gen_string_expr();
    Ptr<Expr> gen_dot_expr(Ptr<Expr> left);
    Ptr<Expr> gen_index_expr(Ptr<Expr> expr);
    Ptr<Expr> gen_enum_expr();
    Ptr<Expr> gen_dict_expr();
    Ptr<Expr> gen_lambda_expr();
        auto gen_parameters() -> LambdaExpr::ParameterList;
    Ptr<Expr> gen_match_expr();
    Ptr<Expr> gen_list_expr();
    Ptr<ClassExpr> gen_class_expr();

  private:
    Tokenizer tokenizer_;
    Token current_token_;
    std::function<void()> resume_action_;
};
}

#endif
