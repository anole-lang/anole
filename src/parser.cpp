#include <set>
#include <exception>
#include <stdexcept>
#include "ast.hpp"
#include "parser.hpp"

#define THROW(MESSAGE) \
    throw runtime_error(get_err_info(MESSAGE))

#define CHECK_AND_THROW(TOKEN_ID, MESSAGE) \
    try_continue(); \
    if (current_token_.token_id != TOKEN_ID) \
        THROW(MESSAGE)

using namespace std;

namespace ice_language
{
Parser::Parser(istream &in, string name_of_in)
  : tokenizer_(in, name_of_in)
{
    get_next_token();
}

void Parser::cont()
{
    tokenizer_.cont();
    get_next_token();
}

void Parser::reset()
{
    tokenizer_.reset();
    get_next_token();
}

void Parser::set_continue_action(
    function<void()> action
)
{
    continue_action_ = move(action);
}

// use when interacting & return stmt node
Ptr<AST> Parser::gen_statement()
{
    return gen_stmt();
}

Ptr<AST> Parser::gen_statements()
{
    return gen_stmts();
}

// update current token when cannot find the next token
Token Parser::get_next_token()
{
    return current_token_ = tokenizer_.next();
}

void Parser::try_continue()
{
    if (current_token_.token_id == TokenId::End
        and AST::interpretive())
    {
        continue_action_();
    }
}

std::string Parser::get_err_info(const string &message)
{
    return tokenizer_.get_err_info(message);
}

ExprList Parser::gen_arguments()
{
    ExprList args;
    get_next_token(); // eat '('
    while (current_token_.token_id != TokenId::RParen)
    {
        args.push_back(gen_delay_expr());
        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token(); // eat ','
        }
        else
        {
            CHECK_AND_THROW(TokenId::RParen, "miss ')'");
        }
    }
    get_next_token(); // eat ')'
    return args;
}

VarDeclList Parser::gen_decl_arguments()
{
    VarDeclList args;
    get_next_token(); // eat '('
    while (current_token_.token_id != TokenId::RParen)
    {
        auto id = dynamic_pointer_cast<IdentifierExpr>(gen_ident());
        Ptr<Expr> expression = make_shared<NoneExpr>();
        if (current_token_.token_id == TokenId::Colon)
        {
            get_next_token();
            expression = gen_expr();
        }
        args.push_back(make_shared<VariableDeclarationStmt>(id, expression));
        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token();
        }
        else
        {
            CHECK_AND_THROW(TokenId::RParen, "miss ')'");
        }
    }
    get_next_token(); // eat ')'
    return args;
}

// usually use when interacting
Ptr<BlockExpr> Parser::gen_stmts()
{
    auto stmts = make_shared<BlockExpr>();
    while (current_token_.token_id != TokenId::End)
    {
        stmts->statements.push_back(gen_stmt());
        if (current_token_.token_id == TokenId::Semicolon)
        {
            get_next_token();
        }
    }
    return stmts;
}

// gen normal block as {...}
Ptr<BlockExpr> Parser::gen_block()
{
    CHECK_AND_THROW(TokenId::LBrace, "missing symbol '{'");
    get_next_token(); // eat '{'
    try_continue();
    auto block = make_shared<BlockExpr>();
    // '}' means the end of a block
    while (current_token_.token_id != TokenId::RBrace)
    {
        auto stmt = gen_stmt();
        if (stmt) block->statements.push_back(stmt);
        if (current_token_.token_id == TokenId::Semicolon)
        {
            get_next_token();
        }
        try_continue();
    }
    get_next_token(); // eat '}'

    return block;
}

Ptr<Expr> Parser::gen_index_expr(Ptr<Expr> expression)
{
    get_next_token();

    auto node = gen_expr();

    CHECK_AND_THROW(TokenId::RBracket, "missing symbol ']'");
    get_next_token();

    return make_shared<IndexExpr>(expression, node);
}

// generate normal statement
Ptr<Stmt> Parser::gen_stmt()
{
    switch (current_token_.token_id)
    {
    // @ is special
    case TokenId::At:
        if (get_next_token().token_id == TokenId::LParen)
        {
            return make_shared<ExprStmt>(gen_lambda_expr());
        }
        else if (current_token_.token_id != TokenId::End)
        {
            return gen_declaration();
        }
        break;

    case TokenId::AtAt:
        return gen_class_decl();

    case TokenId::Using:
        return gen_using_stmt();

    case TokenId::If:
        return gen_if_else();

    case TokenId::While:
        return gen_while_stmt();

    case TokenId::Do:
        return gen_do_while_stmt();

    case TokenId::For:
        return gen_for_stmt();

    case TokenId::Foreach:
        return gen_foreach_stmt();

    case TokenId::Break:
        get_next_token();
        return make_shared<BreakStmt>();

    case TokenId::Continue:
        get_next_token();
        return make_shared<ContinueStmt>();

    case TokenId::Return:
        return gen_return_stmt();

    case TokenId::Identifier:
    case TokenId::Sub:
    case TokenId::Integer:
    case TokenId::Double:
    case TokenId::None:
    case TokenId::True:
    case TokenId::False:
    case TokenId::String:
    case TokenId::LParen:
    case TokenId::New:
    case TokenId::Match:
    case TokenId::LBracket:
    case TokenId::LBrace:
    case TokenId::Not:
        return make_shared<ExprStmt>(gen_expr());

    default:
        break;
    }
    THROW("wrong token here");
}

// generate declaration or assignment (@.var:)
Ptr<Stmt> Parser::gen_declaration()
{
    Ptr<Expr> node = gen_ident();

    switch (current_token_.token_id)
    {
    case TokenId::Colon:
        get_next_token();
        return make_shared<VariableDeclarationStmt>(
            reinterpret_pointer_cast<IdentifierExpr>(node),
            gen_delay_expr()
        );

    case TokenId::LParen:
        {
            auto args = gen_decl_arguments();
            try_continue();
            Ptr<BlockExpr> block = nullptr;
            if (current_token_.token_id == TokenId::Colon)
            {
                get_next_token();
                block = make_shared<BlockExpr>();
                block->statements.push_back(make_shared<ReturnStmt>(gen_expr()));
            }
            else if (current_token_.token_id == TokenId::LBrace)
            {
                block = gen_block();
            }
            else
            {
                THROW("missing symbol ':' or '{' after @()");
            }
            return make_shared<FunctionDeclarationStmt>(
                reinterpret_pointer_cast<IdentifierExpr>(node),
                make_shared<LambdaExpr>(args, block)
            );
        }

    default:
        break;
    }
    return make_shared<VariableDeclarationStmt>(
        reinterpret_pointer_cast<IdentifierExpr>(node),
        nullptr
    );
}

Ptr<Stmt> Parser::gen_class_decl()
{
    get_next_token();

    auto id = dynamic_pointer_cast<IdentifierExpr>(gen_ident());

    IdentList bases;
    CHECK_AND_THROW(TokenId::LParen, "missing symbol '('");
    get_next_token();

    while (current_token_.token_id != TokenId::RParen)
    {
        bases.push_back(dynamic_pointer_cast<IdentifierExpr>(gen_ident()));
        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token();
        }
        else
        {
            CHECK_AND_THROW(TokenId::RParen, "miss ')'");
        }
    }
    get_next_token();

    auto block = gen_block();

    return make_shared<ClassDeclarationStmt>(id, bases, block);
}

Ptr<Stmt> Parser::gen_using_stmt()
{
    get_next_token();
    CHECK_AND_THROW(TokenId::Identifier, "missing an identifier after 'using'");
    auto using_stmt = make_shared<UsingStmt>(current_token_.value);
    get_next_token();
    return using_stmt;
}

Ptr<Stmt> Parser::gen_if_else()
{
    get_next_token();
    auto cond = gen_expr();
    auto block_true = gen_block();
    auto else_stmt = gen_if_else_tail();
    return make_shared<IfElseStmt>(cond, block_true, else_stmt);
}

Ptr<Stmt> Parser::gen_if_else_tail()
{
    if (current_token_.token_id == TokenId::Elif)
    {
        return gen_if_else();
    }
    else if (current_token_.token_id == TokenId::Else)
    {
        get_next_token();
        auto block = gen_block();
        return make_shared<IfElseStmt>(
            make_shared<IntegerExpr>(1), block, nullptr);
    }
    else
    {
        return nullptr;
    }
}

Ptr<Stmt> Parser::gen_while_stmt()
{
    get_next_token();

    auto cond = gen_expr();
    auto block = gen_block();
    return make_shared<WhileStmt>(cond, block);
}

Ptr<Stmt> Parser::gen_do_while_stmt()
{
    get_next_token();

    auto block = gen_block();

    CHECK_AND_THROW(TokenId::While, "missing keyword 'while' after 'do'");
    get_next_token();

    auto cond = gen_expr();
    return make_shared<DoWhileStmt>(cond, block);
}

Ptr<Stmt> Parser::gen_for_stmt()
{
    get_next_token();

    auto begin = gen_expr();

    CHECK_AND_THROW(TokenId::To, "missing keyword 'to' in for");
    get_next_token();

    auto end = gen_expr();
    shared_ptr<IdentifierExpr> id = nullptr;
    if (current_token_.token_id == TokenId::As)
    {
        get_next_token();
        id = dynamic_pointer_cast<IdentifierExpr>(gen_ident());
    }
    auto block = gen_block();

    return make_shared<ForStmt>(begin, end, id, block);
}

Ptr<Stmt> Parser::gen_foreach_stmt()
{
    get_next_token();

    auto expression = gen_expr();

    CHECK_AND_THROW(TokenId::As, "missing keyword 'as' in foreach");
    get_next_token();

    auto id = dynamic_pointer_cast<IdentifierExpr>(gen_ident());
    auto block = gen_block();

    return make_shared<ForeachStmt>(expression, id, block);
}

Ptr<Stmt> Parser::gen_return_stmt()
{
    get_next_token();
    return make_shared<ReturnStmt>(gen_expr());
}

Ptr<Expr> Parser::gen_delay_expr()
{
    try_continue();
    if (current_token_.token_id == TokenId::Delay)
    {
        get_next_token();
        return make_shared<DelayExpr>(gen_expr());
    }
    else
    {
        return gen_expr();
    }
}

static const vector<set<TokenId>> &get_operators()
{
    static const vector<set<TokenId>> operators
    {
        { TokenId::Or },
        { TokenId::And },
        { TokenId::CEQ, TokenId::CNE, TokenId::CLT, TokenId::CLE, TokenId::CGT, TokenId::CGE },
        { TokenId::Add, TokenId::Sub },
        { TokenId::Mul, TokenId::Div, TokenId::Mod },
        { TokenId::Not, TokenId::Sub }
    };
    return operators;
}

Ptr<Expr> Parser::gen_expr(int priority)
{
    if (priority == -1)
    {
        auto expr = gen_expr(0);
        if (current_token_.token_id == TokenId::Ques)
        {
            get_next_token();
            auto true_expr = gen_expr();
            CHECK_AND_THROW(TokenId::Comma, "miss , here");
            get_next_token();
            auto false_expr = gen_expr();
            expr = make_shared<QuesExpr>(expr, true_expr, false_expr);
        }
        return expr;
    }

    if (priority == get_operators().size() - 1)
    {
        if (get_operators()[priority].count(current_token_.token_id))
        {
            auto op = current_token_.token_id;
            get_next_token();
            try_continue();
            return make_shared<UnaryOperatorExpr>(
                op, gen_expr(priority)
            );
        }
        else
        {
            return gen_term_tail(gen_term());
        }
    }

    auto lhs = gen_expr(priority + 1);
    auto op = current_token_.token_id;
    if (get_operators()[priority].count(op))
    {
        get_next_token();
        lhs = make_shared<BinaryOperatorExpr>(lhs, op, gen_expr(priority));
    }
    return lhs;
}

Ptr<Expr> Parser::gen_term()
{
    try_continue();
    Ptr<Expr> node = nullptr;
    switch (current_token_.token_id)
    {
    case TokenId::Identifier:
        return gen_ident();

    case TokenId::Integer:
    case TokenId::Double:
        return gen_numeric();

    case TokenId::None:
        return gen_none();

    case TokenId::True:
    case TokenId::False:
        return gen_boolean();

    case TokenId::String:
        return gen_string();

    case TokenId::LParen:
        get_next_token();
        node = gen_expr();
        CHECK_AND_THROW(TokenId::RParen, "missing symbol ')' .");
        get_next_token();
        return node;

    case TokenId::At:
        get_next_token();
        return gen_lambda_expr();

    case TokenId::New:
        return gen_new_expr();

    case TokenId::Match:
        return gen_match_expr();

    case TokenId::LBracket:
        return gen_list_expr();

    case TokenId::LBrace:
        return gen_enum_or_dict();

    default:
        THROW("miss an expr behind here");
    }
}

Ptr<Expr> Parser::gen_term_tail(Ptr<Expr> expr)
{
    try_continue();
    while (current_token_.token_id == TokenId::Dot
      || current_token_.token_id == TokenId::LParen
      || current_token_.token_id == TokenId::LBracket)
    {
        if (current_token_.token_id == TokenId::Dot)
        {
            expr = gen_dot_expr(expr);
        }
        else if (current_token_.token_id == TokenId::LParen)
        {
            expr = make_shared<ParenOperatorExpr>(expr, gen_arguments());
        }
        else // if Token is LBracket
        {
            expr = gen_index_expr(expr);
        }
        try_continue();
    }

    if (current_token_.token_id == TokenId::Colon)
    {
        get_next_token();
        return make_shared<BinaryOperatorExpr>(
            expr, TokenId::Colon, gen_delay_expr());
    }

    return expr;
}

Ptr<Expr> Parser::gen_ident()
{
    auto ident_expr = make_shared<IdentifierExpr>(current_token_.value);
    get_next_token();
    return ident_expr;
}

Ptr<Expr> Parser::gen_numeric()
{
    Ptr<Expr> numeric_expr = nullptr;
    if (current_token_.token_id == TokenId::Integer)
    {
        numeric_expr = make_shared<IntegerExpr>(stoi(current_token_.value));
    }
    else
    {
        numeric_expr = make_shared<FloatExpr>(stod(current_token_.value));
    }
    get_next_token();
    return numeric_expr;
}

Ptr<Expr> Parser::gen_none()
{
    get_next_token();
    return make_shared<NoneExpr>();
}

Ptr<Expr> Parser::gen_boolean()
{
    auto bool_expr = make_shared<BoolExpr>(
        ((current_token_.token_id == TokenId::True)
          ? true : false));
    get_next_token();
    return bool_expr;
}

Ptr<Expr> Parser::gen_string()
{
    auto string_expr = make_shared<StringExpr>(current_token_.value);
    get_next_token();
    return string_expr;
}

Ptr<Expr> Parser::gen_dot_expr(Ptr<Expr> left)
{
    get_next_token();
    return make_shared<DotExpr>(left, gen_ident());
}

// generate enum as { NAME1, NAME2, ..., NAMEN }
// or dict as {KEY1: VAL1, KEY2: VAL2, ..., KEYN: VALN}
Ptr<Expr> Parser::gen_enum_or_dict()
{
    get_next_token();

    if (current_token_.token_id == TokenId::RBrace)
    {
        get_next_token();
        return make_shared<DictExpr>();
    }
    auto node = gen_expr();
    if (current_token_.token_id == TokenId::Ret)
    {
        node = gen_dict_expr(node);
    }
    else
    {
        node = gen_enum_expr(node);
    }
    CHECK_AND_THROW(TokenId::RBrace, "missing symbol '}'");
    get_next_token();
    return node;
}

Ptr<Expr> Parser::gen_enum_expr(Ptr<Expr> first)
{
    IdentList enumerators;
    enumerators.push_back(dynamic_pointer_cast<IdentifierExpr>(first));
    if (current_token_.token_id == TokenId::RBrace)
    {
        return make_shared<EnumExpr>(enumerators);
    }
    get_next_token();
    while (current_token_.token_id == TokenId::Identifier)
    {
        enumerators.push_back(dynamic_pointer_cast<IdentifierExpr>(gen_ident()));
        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token();
            CHECK_AND_THROW(TokenId::Identifier, "miss identifier");
        }
        else
        {
            CHECK_AND_THROW(TokenId::RBrace, "miss '}'");
        }
    }
    return make_shared<EnumExpr>(enumerators);
}

Ptr<Expr> Parser::gen_dict_expr(Ptr<Expr> first)
{
    CHECK_AND_THROW(TokenId::Ret, "missing symbol ':'");
    ExprList keys, values;
    keys.push_back(first);

    get_next_token();

    values.push_back(gen_expr());
    if (current_token_.token_id == TokenId::Comma) get_next_token();
    while (current_token_.token_id != TokenId::RBrace)
    {
        keys.push_back(gen_expr());

        CHECK_AND_THROW(TokenId::Ret, "missing symbol '=>'");
        get_next_token();

        values.push_back(gen_expr());
        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token();
        }
        else
        {
            CHECK_AND_THROW(TokenId::RBrace, "miss '}'");
        }
    }
    return make_shared<DictExpr>(keys, values);
}

// generate lambda expr as @(): expr, @(){} and also @(){}()..
Ptr<Expr> Parser::gen_lambda_expr()
{
    auto args = gen_decl_arguments();
    try_continue();
    Ptr<BlockExpr> block = nullptr;

    if (current_token_.token_id == TokenId::Colon)
    {
        get_next_token();
        block = make_shared<BlockExpr>();
        block->statements.push_back(make_shared<ReturnStmt>(gen_expr()));
    }
    else if (current_token_.token_id == TokenId::LBrace)
    {
        block = gen_block();
    }
    else
    {
        THROW("missing symbol ':' or '{' after @()");
    }

    Ptr<Expr> node = make_shared<LambdaExpr>(args, block);
    try_continue();
    while (current_token_.token_id == TokenId::LParen)
    {
        node = make_shared<ParenOperatorExpr>(node, gen_arguments());
        try_continue();
    }

    return node;
}

// generate new expr as @instance: new Class()
Ptr<Expr> Parser::gen_new_expr()
{
    get_next_token();

    auto id = dynamic_pointer_cast<IdentifierExpr>(gen_ident());
    CHECK_AND_THROW(TokenId::LParen, "missing symbol '('");
    ExprList args = gen_arguments();

    return make_shared<NewExpr>(id, args);
}

/* Ptr<Expr> SyntaxAnalyzer::gen_match_expr()
generate match expr as follow:
    match value {
        1, 2, 3, 4 => "smaller than five",
        5 => "five"
    } else "bigger than five"
*/
Ptr<Expr> Parser::gen_match_expr()
{
    auto match_expr = make_shared<MatchExpr>();

    get_next_token(); // eat 'match'
    match_expr->expr = gen_expr();

    CHECK_AND_THROW(TokenId::LBrace, "missing symbol '{'");
    get_next_token(); // eat '{'

    while (current_token_.token_id != TokenId::RBrace)
    {
        match_expr->keylists.push_back({});
        match_expr->keylists.back().push_back(gen_expr());
        while (current_token_.token_id == TokenId::Comma)
        {
            try_continue();
            get_next_token();
            match_expr->keylists.back().push_back(gen_expr());
        }

        CHECK_AND_THROW(TokenId::Ret, "missing symbol '=>'");
        get_next_token();

        match_expr->values.push_back(gen_expr());

        if (current_token_.token_id == TokenId::Comma)
        {
            get_next_token();
        }
        else
        {
            CHECK_AND_THROW(TokenId::RBrace, "miss '}'");
        }
    }

    get_next_token(); // eat '}'

    // check 'else' after 'match {}'
    if (current_token_.token_id == TokenId::Else)
    {
        get_next_token();
        match_expr->else_expr = gen_expr();
    }
    else
    {
        match_expr->else_expr = nullptr;
    }

    return match_expr;
}

// generate list as [expr1, expr2, ..., exprN]
Ptr<Expr> Parser::gen_list_expr()
{
    get_next_token(); // eat '['

    ExprList expressions;
    while (current_token_.token_id != TokenId::RBracket)
    {
        expressions.push_back(gen_expr());
        if (current_token_.token_id == TokenId::Comma) get_next_token();
    }

    get_next_token(); // eat(']')

    return make_shared<ListExpr>(expressions);
}
}
