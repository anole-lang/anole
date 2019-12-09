#pragma once

#include <memory>
#include <vector>
#include <istream>
#include <iostream>
#include <iterator>
#include "ast.hpp"
#include "tokenizer.hpp"

namespace ice_language
{
using ASTPtr       = Ptr<struct AST>;
using ExprPtr      = Ptr<struct Expr>;
using StmtPtr      = Ptr<struct Stmt>;
using BlockExprPtr = Ptr<struct BlockExpr>;

class Parser
{
  public:
    Parser(std::istream &in = std::cin);
    Ptr<AST> gen_ast();

  private:
    Token current_token_;
    Tokenizer tokenizer_;
    Token get_next_token();
    std::string get_location();

    ExprList gen_arguments();
    VarDeclList gen_decl_arguments();

    BlockExprPtr gen_stmts();
    BlockExprPtr gen_block();

    StmtPtr gen_stmt();
    StmtPtr gen_decl_or_assign();
    StmtPtr gen_var_assign();
    StmtPtr gen_class_decl();
    StmtPtr gen_using_stmt();
    StmtPtr gen_if_else();
    StmtPtr gen_if_else_tail();
    StmtPtr gen_while_stmt();
    StmtPtr gen_do_while_stmt();
    StmtPtr gen_for_stmt();
    StmtPtr gen_foreach_stmt();
    StmtPtr gen_return_stmt();

    ExprPtr gen_expr(int priority = 0);
    ExprPtr gen_term();
    ExprPtr gen_term_tail(ExprPtr expr);
    ExprPtr gen_ident();
    ExprPtr gen_numeric();
    ExprPtr gen_none();
    ExprPtr gen_boolean();
    ExprPtr gen_string();
    ExprPtr gen_dot_expr(ExprPtr left);
    ExprPtr gen_index_expr(ExprPtr expr);
    ExprPtr gen_enum_or_dict();
    ExprPtr gen_enum_expr(ExprPtr first);
    ExprPtr gen_dict_expr(ExprPtr first);
    ExprPtr gen_lambda_expr();
    ExprPtr gen_new_expr();
    ExprPtr gen_match_expr();
    ExprPtr gen_list_expr();
};
}
