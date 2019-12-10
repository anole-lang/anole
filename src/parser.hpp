#pragma once

#include <vector>
#include <istream>
#include <iostream>
#include <iterator>
#include "helper.hpp"
#include "ast.hpp"
#include "tokenizer.hpp"

namespace ice_language
{
class Parser
{
  public:
    Parser(std::istream &in = std::cin);
    Ptr<AST> gen_statement();
    Ptr<AST> gen_statements();

  private:
    Token current_token_;
    Tokenizer tokenizer_;
    Token get_next_token();
    std::string get_err_info(const std::string &message);

    ExprList gen_arguments();
    VarDeclList gen_decl_arguments();

    Ptr<BlockExpr> gen_stmts();
    Ptr<BlockExpr> gen_block();

    Ptr<Stmt> gen_stmt();
    Ptr<Stmt> gen_declaration();
    Ptr<Stmt> gen_var_assign();
    Ptr<Stmt> gen_class_decl();
    Ptr<Stmt> gen_using_stmt();
    // should be rewrited
    Ptr<Stmt> gen_if_else();
    Ptr<Stmt> gen_if_else_tail();
    Ptr<Stmt> gen_while_stmt();
    Ptr<Stmt> gen_do_while_stmt();
    Ptr<Stmt> gen_for_stmt();
    Ptr<Stmt> gen_foreach_stmt();
    Ptr<Stmt> gen_return_stmt();

    Ptr<Expr> gen_delay_expr();
    Ptr<Expr> gen_expr(int priority = -1);
    Ptr<Expr> gen_term();
    Ptr<Expr> gen_term_tail(Ptr<Expr> expr);
    Ptr<Expr> gen_ident();
    Ptr<Expr> gen_numeric();
    Ptr<Expr> gen_none();
    Ptr<Expr> gen_boolean();
    Ptr<Expr> gen_string();
    Ptr<Expr> gen_dot_expr(Ptr<Expr> left);
    Ptr<Expr> gen_index_expr(Ptr<Expr> expr);
    Ptr<Expr> gen_enum_or_dict();
    Ptr<Expr> gen_enum_expr(Ptr<Expr> first);
    Ptr<Expr> gen_dict_expr(Ptr<Expr> first);
    Ptr<Expr> gen_lambda_expr();
    Ptr<Expr> gen_new_expr();
    Ptr<Expr> gen_match_expr();
    Ptr<Expr> gen_list_expr();
};
}
