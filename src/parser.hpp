#pragma once

#include <vector>
#include <istream>
#include <iostream>
#include <iterator>
#include <functional>
#include "ast.hpp"
#include "tokenizer.hpp"

namespace anole
{
class Parser
{
  public:
    Parser(std::istream & = std::cin,
        std::string = "<stdin>");
    void cont();
    void reset();
    void set_continue_action(std::function<void()>);
    Ptr<AST> gen_statement();
    Ptr<AST> gen_statements();

    static void add_infixop(const std::string &str,
        std::size_t priority);

  private:
    Token current_token_;
    Tokenizer tokenizer_;
    std::function<void()> continue_action_;

    void throw_err(const std::string &err_info);

    template <TokenType type>
    void check(const std::string &err_info)
    {
        try_continue();
        if (current_token_.type != type)
        {
            throw_err(err_info);
        }
    }

    template <TokenType type>
    void eat(const std::string &err_info = "")
    {
        check<type>(err_info);
        get_next_token();
    }

    Token &get_next_token();
    void try_continue();
    std::string get_err_info(const std::string &message);

    ExprList gen_arguments();
    IdentList gen_idents();
    DeclList gen_arg_decls();

    Ptr<BlockExpr> gen_stmts();
    Ptr<BlockExpr> gen_block();

    Ptr<Stmt> gen_stmt();
    Ptr<Stmt> gen_declaration();
    Ptr<Stmt> gen_var_assign();
    Ptr<Stmt> gen_infixop_decl();
    Ptr<Stmt> gen_class_decl();
    Ptr<Stmt> gen_use_stmt();
    // should be rewrited
    Ptr<Stmt> gen_if_else();
    Ptr<Stmt> gen_if_else_tail();
    Ptr<Stmt> gen_while_stmt();
    Ptr<Stmt> gen_do_while_stmt();
    Ptr<Stmt> gen_foreach_stmt();
    Ptr<Stmt> gen_return_stmt();

    Ptr<Expr> gen_delay_expr();
    Ptr<Expr> gen_expr(int layer = -1);
    Ptr<Expr> gen_term();
    Ptr<Expr> gen_term_tail(Ptr<Expr> expr);
    Ptr<IdentifierExpr> gen_ident();
    Ptr<Expr> gen_numeric();
    Ptr<Expr> gen_none();
    Ptr<Expr> gen_boolean();
    Ptr<Expr> gen_string();
    Ptr<Expr> gen_dot_expr(Ptr<Expr> left);
    Ptr<Expr> gen_index_expr(Ptr<Expr> expr);
    Ptr<Expr> gen_enum_expr();
    Ptr<Expr> gen_dict_expr();
    Ptr<Expr> gen_lambda_expr();
    Ptr<Expr> gen_new_expr();
    Ptr<Expr> gen_match_expr();
    Ptr<Expr> gen_list_expr();
};
}
