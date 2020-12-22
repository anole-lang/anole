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
    Parser() noexcept;
    Parser(std::istream &input, String name_of_input) noexcept;

    void resume();
    void reset();
    void set_resume_action(std::function<void()>);

    Ptr<AST> gen_statement();
    Ptr<AST> gen_statements();

  private:
    CompileError parse_error(const String &err_info);
    template<TokenType type>
    void check(const String &err_info)
    {
        try_resume();
        if (current_token_.type != type)
        {
            throw parse_error(err_info);
        }
    }
    template<TokenType type>
    void eat(const String &err_info = "")
    {
        check<type>(err_info);
        next_token();
    }

    void get_next_token();
    Token &next_token();
    void try_resume(Size times = 0);

    ArgumentList gen_arguments();
    ParameterList gen_parameters();

    Ptr<BlockExpr> gen_stmts();
    Ptr<BlockExpr> gen_block();

    Ptr<Stmt> gen_stmt();
    Ptr<DeclarationStmt> gen_declaration();
    Ptr<DeclarationStmt> gen_class_declaration();
    Ptr<Stmt> gen_use_stmt();
        UseStmt::Module gen_module();
        UseStmt::Alias gen_alias();
    Ptr<Stmt> gen_prefixop_decl();
    Ptr<Stmt> gen_infixop_decl();
    Ptr<Stmt> gen_if_else();
    Ptr<AST>  gen_if_else_tail();
    Ptr<Stmt> gen_while_stmt();
    Ptr<Stmt> gen_do_while_stmt();
    Ptr<Stmt> gen_foreach_stmt();
    Ptr<Stmt> gen_return_stmt();

    Ptr<Expr> gen_delay_expr();
    Ptr<Expr> gen_expr(int layer = -1);
    Ptr<Expr> gen_term();
    Ptr<Expr> gen_term_tail(Ptr<Expr> expr);
    String gen_ident_rawstr();
    Ptr<IdentifierExpr> gen_ident_expr();
    Ptr<Expr> gen_numeric_expr();
    Ptr<Expr> gen_none_expr();
    Ptr<Expr> gen_boolean_expr();
    Ptr<Expr> gen_string_expr();
    Ptr<Expr> gen_dot_expr(Ptr<Expr> left);
    Ptr<Expr> gen_index_expr(Ptr<Expr> expr);
    Ptr<Expr> gen_enum_expr();
    Ptr<Expr> gen_dict_expr();
    Ptr<ClassExpr> gen_class_expr();
    Ptr<Expr> gen_lambda_expr();
    Ptr<Expr> gen_match_expr();
    Ptr<Expr> gen_list_expr();

  private:
    Tokenizer tokenizer_;
    Token current_token_;
    std::function<void()> resume_action_;
};
}

#endif
