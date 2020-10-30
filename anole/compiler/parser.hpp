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
    Parser(std::istream & = std::cin,
        String = "<stdin>"
    );
    void cont();
    void reset();
    void set_continue_action(std::function<void()>);
    Ptr<AST> gen_statement();
    Ptr<AST> gen_statements();

    // add new prefix operator
    static void add_prefixop(const String &str);
    // add new infix operator with priority
    static void add_infixop(
        const String &str,
        Size priority
    );

  private:
    Token current_token_;
    Tokenizer tokenizer_;
    std::function<void()> continue_action_;

    CompileError parse_error(const String &err_info);

    template<TokenType type>
    void check(const String &err_info)
    {
        try_continue();
        if (current_token_.type != type)
        {
            throw parse_error(err_info);
        }
    }

    template<TokenType type>
    void eat(const String &err_info = "")
    {
        check<type>(err_info);
        get_next_token();
    }

    Token &get_next_token();
    void try_continue();
    String get_err_info(const String &message);

    ArgumentList gen_arguments();
    ParameterList gen_parameters();

    Ptr<BlockExpr> gen_stmts();
    Ptr<BlockExpr> gen_block();

    Ptr<Stmt> gen_stmt();
    Ptr<DeclarationStmt> gen_declaration();
    Ptr<Stmt> gen_prefixop_decl();
    Ptr<Stmt> gen_infixop_decl();
    UseStmt::Module gen_module();
    UseStmt::Alias gen_alias();
    Ptr<Stmt> gen_use_stmt();
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
    Ptr<Expr> gen_class_expr();
    Ptr<Expr> gen_lambda_expr();
    Ptr<Expr> gen_match_expr();
    Ptr<Expr> gen_list_expr();
};
}

#endif
