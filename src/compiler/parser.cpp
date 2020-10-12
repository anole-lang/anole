#include "compiler.hpp"

#include "../error.hpp"

#include <set>
#include <algorithm>

using namespace std;

namespace anole
{
Parser::Parser(istream &in, String name_of_in)
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
    function<void()> action)
{
    continue_action_ = move(action);
}

// use when interacting & return stmt node
Ptr<AST> Parser::gen_statement()
{
    auto stmt = gen_stmt();
    if (current_token_.type == TokenType::Semicolon)
    {
        get_next_token();
    }
    return stmt;
}

Ptr<AST> Parser::gen_statements()
{
    return gen_stmts();
}

namespace operators
{
set<TokenType> unary_ops
{
    TokenType::Not, TokenType::Sub, TokenType::BNeg
};

vector<Size> bop_priorities
{
    100, 110, 120, 130, 140, 150, 160, 170, 180, 190
};
map<Size, set<TokenType>> bop_mapping
{
    { 100, { TokenType::Or } },
    { 110, { TokenType::And } },
    { 120, { TokenType::BOr } },
    { 130, { TokenType::BXor } },
    { 140, { TokenType::BAnd } },
    { 150, { TokenType::CEQ, TokenType::CNE } },
    { 160, { TokenType::CLT, TokenType::CLE, TokenType::CGT, TokenType::CGE } },
    { 170, { TokenType::BLS, TokenType::BRS } },
    { 180, { TokenType::Add, TokenType::Sub } },
    { 190, { TokenType::Is,  TokenType::Mul, TokenType::Div, TokenType::Mod } }
};

set<TokenType> &bops_at_priority(Size priority)
{
    if (!bop_mapping.count(priority))
    {
        bop_mapping[priority] = {};
    }
    return bop_mapping[priority];
}

set<TokenType> &bops_at_layer(Size layer)
{
    return bop_mapping[bop_priorities[layer]];
}
}

void Parser::add_prefixop(const String &str)
{
    auto type = Token::add_token_type(str);
    if (type <= TokenType::End)
    {
        throw RuntimeError("can't define predefined keywords or operators");
    }

    operators::unary_ops.insert(type);
}

void Parser::add_infixop(const String &str, Size priority)
{
    auto type = Token::add_token_type(str);
    if (type <= TokenType::End)
    {
        throw RuntimeError("can't define predefined keywords or operators");
    }

    auto lower = lower_bound(operators::bop_priorities.begin(),
        operators::bop_priorities.end(), priority
    );
    if (lower == operators::bop_priorities.end() || *lower != priority)
    {
        operators::bop_priorities.insert(lower, priority);
    }

    operators::bops_at_priority(priority).insert(type);
}

CompileError Parser::parse_error(const String &err_info)
{
    return CompileError(get_err_info(err_info));
}

// update current token when cannot find the next token
Token &Parser::get_next_token()
{
    return current_token_ = tokenizer_.next();
}

void Parser::try_continue()
{
    if (current_token_.type == TokenType::End
        && AST::interpretive())
    {
        continue_action_();
    }
}

String Parser::get_err_info(const String &message)
{
    return tokenizer_.get_err_info(message);
}

IdentList Parser::gen_idents()
{
    IdentList idents;
    while (current_token_.type != TokenType::RParen)
    {
        idents.push_back(gen_ident_expr());
        if (current_token_.type == TokenType::Comma)
        {
            get_next_token();
        }
        else
        {
            check<TokenType::RParen>("expected ')' here");
        }
    }
    return idents;
}

ArgumentList Parser::gen_arguments()
{
    ArgumentList args;
    get_next_token(); // eat '('
    while (current_token_.type != TokenType::RParen)
    {
        auto expr = gen_delay_expr();
        bool unpack = false;
        if (current_token_.type == TokenType::Dooot)
        {
            unpack = true;
            get_next_token();
        }
        args.push_back(make_pair(move(expr), unpack));
        if (current_token_.type == TokenType::Comma)
        {
            get_next_token(); // eat ','
        }
        else
        {
            check<TokenType::RParen>("expected ')'");
        }
    }
    get_next_token(); // eat ')'
    return args;
}

ParameterList Parser::gen_parameters()
{
    bool need_default = false;
    bool packed = false;

    ParameterList parameters;
    while (current_token_.type != TokenType::RParen)
    {
        if (current_token_.type == TokenType::Dooot)
        {
            packed = true;
            get_next_token();

            bool is_ref = false;
            if (current_token_.type == TokenType::BAnd)
            {
                is_ref = true;
                get_next_token();
            }

            auto decl = make_unique<VariableDeclarationStmt>(
                gen_ident_expr(), nullptr, is_ref
            );
            parameters.push_back(make_pair(move(decl), true));
        }
        else
        {
            bool is_ref = false;
            if (current_token_.type == TokenType::BAnd)
            {
                is_ref = true;
                get_next_token();
            }

            auto ident = gen_ident_expr();
            Ptr<VariableDeclarationStmt> decl = nullptr;
            if (current_token_.type == TokenType::Colon)
            {
                need_default = true;
                get_next_token();
                decl = make_unique<VariableDeclarationStmt>(
                    move(ident), gen_expr(), is_ref
                );
            }
            else if (need_default)
            {
                throw parse_error("parameter without default argument cannot follow parameter with default argument");
            }
            else
            {
                decl = make_unique<VariableDeclarationStmt>(
                    move(ident), nullptr, is_ref
                );
            }

            parameters.push_back(make_pair(move(decl), false));
        }

        if (current_token_.type == TokenType::Comma)
        {
            if (packed)
            {
                throw parse_error("packed parameter should be the last parameter");
            }
            get_next_token();
        }
        else
        {
            check<TokenType::RParen>("expected ')' here");
        }
    }
    return parameters;
}

// usually use when interacting
Ptr<BlockExpr> Parser::gen_stmts()
{
    auto stmts = make_unique<BlockExpr>();
    while (current_token_.type != TokenType::End)
    {
        stmts->statements.push_back(gen_stmt());
        while (current_token_.type == TokenType::Semicolon)
        {
            get_next_token();
        }
    }
    return stmts;
}

// gen normal block as {...}
Ptr<BlockExpr> Parser::gen_block()
{
    Ptr<BlockExpr> block;

    if (current_token_.type == TokenType::LBrace)
    {
        get_next_token(); // eat '{'
        try_continue();
        block = make_unique<BlockExpr>();
        // '}' means the end of a block
        while (current_token_.type != TokenType::RBrace)
        {
            auto stmt = gen_stmt();
            if (stmt)
            {
                block->statements.push_back(move(stmt));
            }

            if (current_token_.type == TokenType::Semicolon)
            {
                get_next_token();
            }
            try_continue();
        }
        get_next_token(); // eat '}'
    }
    else if (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        try_continue();
        block = make_unique<BlockExpr>();
        block->statements.push_back(gen_stmt());
        if (current_token_.type == TokenType::Semicolon)
        {
            get_next_token();
        }
    }
    else
    {
        throw parse_error("expected '{' or ',' here");
    }

    return block;
}

Ptr<Expr> Parser::gen_index_expr(Ptr<Expr> expression)
{
    auto pos = tokenizer_.last_pos();
    get_next_token();

    auto node = gen_expr();

    check<TokenType::RBracket>("expected ']'");
    get_next_token();

    auto index = make_unique<IndexExpr>(move(expression), move(node));
    index->pos = pos;
    return index;
}

// generate normal statement
Ptr<Stmt> Parser::gen_stmt()
{
    switch (current_token_.type)
    {
    case TokenType::End:
        return nullptr;

    // @ is special
    case TokenType::At:
        if (get_next_token().type == TokenType::LParen)
        {
            return make_unique<ExprStmt>(gen_lambda_expr());
        }
        else if (current_token_.type != TokenType::End)
        {
            return gen_declaration();
        }
        break;

    case TokenType::Use:
        return gen_use_stmt();

    case TokenType::Prefixop:
        return gen_prefixop_decl();

    case TokenType::Infixop:
        return gen_infixop_decl();

    case TokenType::If:
        return gen_if_else();

    case TokenType::While:
        return gen_while_stmt();

    case TokenType::Do:
        return gen_do_while_stmt();

    case TokenType::Foreach:
        return gen_foreach_stmt();

    case TokenType::Break:
        get_next_token();
        return make_unique<BreakStmt>();

    case TokenType::Continue:
        get_next_token();
        return make_unique<ContinueStmt>();

    case TokenType::Return:
        return gen_return_stmt();

    case TokenType::Identifier:
    case TokenType::Sub:
    case TokenType::Integer:
    case TokenType::Double:
    case TokenType::None:
    case TokenType::True:
    case TokenType::False:
    case TokenType::String:
    case TokenType::LParen:
    case TokenType::Enum:
    case TokenType::Dict:
    case TokenType::Class:
    case TokenType::Match:
    case TokenType::LBracket:
    case TokenType::LBrace:
    case TokenType::Not:
        return make_unique<ExprStmt>(gen_expr());

    default:
        if (operators::unary_ops.count(current_token_.type))
        {
            return make_unique<ExprStmt>(gen_expr());
        }
        break;
    }
    throw parse_error("wrong token here");
}

// generate declaration or assignment (@var:)
Ptr<Stmt> Parser::gen_declaration()
{
    bool is_ref = false;
    if (current_token_.type == TokenType::BAnd)
    {
        is_ref = true;
        get_next_token();
    }

    auto id = gen_ident_expr();
    switch (current_token_.type)
    {
    case TokenType::Comma:
    {
        list<VariableDeclarationStmt> decls;
        decls.emplace_back(move(id), nullptr, is_ref);

        while (current_token_.type == TokenType::Comma)
        {
            get_next_token();
            bool is_ref = false;
            if (current_token_.type == TokenType::BAnd)
            {
                is_ref = true;
                get_next_token();
            }
            decls.emplace_back(gen_ident_expr(), nullptr, is_ref);
        }

        // @var1, ..., varn
        if (current_token_.type != TokenType::Colon)
        {
            return make_unique<MultiVarsDeclarationStmt>(move(decls), ExprList{});
        }

        // @var1, ..., varn: expr
        get_next_token();
        ExprList exprs;
        exprs.push_back(gen_delay_expr());
        while (current_token_.type == TokenType::Comma)
        {
            get_next_token();
            exprs.push_back(gen_delay_expr());
        }
        return make_unique<MultiVarsDeclarationStmt>(move(decls), move(exprs));
    }

    case TokenType::Colon:
        get_next_token();
        return make_unique<VariableDeclarationStmt>(
            move(id), gen_delay_expr(), is_ref
        );

    case TokenType::LParen:
    {
        if (is_ref)
        {
            throw parse_error("& cannot be here");
        }

        get_next_token();
        auto parameters = gen_parameters();
        get_next_token();

        try_continue();
        Ptr<BlockExpr> block = nullptr;

        if (current_token_.type == TokenType::Colon)
        {
            get_next_token();
            block = make_unique<BlockExpr>();
            ExprList exprs;
            exprs.push_back(gen_delay_expr());
            while (current_token_.type == TokenType::Comma)
            {
                get_next_token();
                exprs.push_back(gen_delay_expr());
            }

            block->statements.push_back(make_unique<ReturnStmt>(move(exprs)));
        }
        else
        {
            block = gen_block();
        }
        return make_unique<FunctionDeclarationStmt>(move(id),
            make_unique<LambdaExpr>(move(parameters), move(block))
        );
    }

    default:
        if (is_ref)
        {
            throw parse_error("reference should be binded with other variable");
        }
        break;
    }
    return make_unique<VariableDeclarationStmt>(move(id), make_unique<NoneExpr>());
}

Ptr<Stmt> Parser::gen_prefixop_decl()
{
    get_next_token();
    check<TokenType::Identifier>("expected an identifier here");
    return make_unique<PrefixopDeclarationStmt>(gen_ident_expr());
}

Ptr<Stmt> Parser::gen_infixop_decl()
{
    get_next_token();
    Size priority = 50;
    if (current_token_.type == TokenType::Integer)
    {
        priority = stoull(current_token_.value);
        get_next_token();
    }
    check<TokenType::Identifier>("expected an identifier here");
    return make_unique<InfixopDeclarationStmt>(gen_ident_expr(), priority);
}

UseStmt::Module Parser::gen_module()
{
    UseStmt::Module mod;
    if (current_token_.type == TokenType::Identifier)
    {
        mod = { current_token_.value, UseStmt::Module::Type::Name };
    }
    else if (current_token_.type == TokenType::String)
    {
        mod = { current_token_.value, UseStmt::Module::Type::Path };
    }
    else
    {
        throw parse_error("need name or path of the source module after from");
    }
    // eat `<module>`
    get_next_token();
    return mod;
}

UseStmt::Alias Parser::gen_alias()
{
    UseStmt::Alias alias;
    alias.first = gen_module();
    if (current_token_.type == TokenType::As)
    {
        get_next_token();
        check<TokenType::Identifier>("need the alias here");
        alias.second = current_token_.value;
        get_next_token();
    }
    else
    {
        if (alias.first.type == UseStmt::Module::Type::Path)
        {
            throw parse_error("use direct path of module need an alias");
        }
        else
        {
            alias.second = alias.first.mod;
        }
    }
    return alias;
}

Ptr<Stmt> Parser::gen_use_stmt()
{
    UseStmt::Aliases aliases;
    UseStmt::Module  from;

    // eat `use`
    get_next_token();

    // use * from <module>
    if (current_token_.type == TokenType::Mul)
    {
        get_next_token();
        eat<TokenType::From>("need from ident here");
        return make_unique<UseStmt>(move(aliases), gen_module());
    }

    aliases.push_back(gen_alias());
    bool use_direct = aliases.back().first.type == UseStmt::Module::Type::Path;
    while (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        aliases.push_back(gen_alias());
        use_direct |= aliases.back().first.type == UseStmt::Module::Type::Path;
    }

    if (current_token_.type == TokenType::From)
    {
        if (use_direct)
        {
            throw parse_error("path of module cannot appear before from");
        }
        get_next_token();
        from = gen_module();
    }
    else
    {
        from.type = UseStmt::Module::Type::Null;
    }

    return make_unique<UseStmt>(move(aliases), move(from));
}

Ptr<Stmt> Parser::gen_if_else()
{
    get_next_token();
    auto pos = tokenizer_.last_pos();
    auto cond = gen_expr();
    cond->pos = pos;
    auto true_block = gen_block();
    try_continue();
    auto false_branch = gen_if_else_tail();
    return make_unique<IfElseStmt>(move(cond),
        move(true_block), move(false_branch)
    );
}

Ptr<AST> Parser::gen_if_else_tail()
{
    if (current_token_.type == TokenType::Elif)
    {
        return gen_if_else();
    }
    else if (current_token_.type == TokenType::Else)
    {
        get_next_token();
        return gen_block();
    }
    else
    {
        return nullptr;
    }
}

Ptr<Stmt> Parser::gen_while_stmt()
{
    get_next_token();
    auto pos = tokenizer_.last_pos();
    auto cond = gen_expr();
    cond->pos = pos;
    auto block = gen_block();
    return make_unique<WhileStmt>(move(cond), move(block));
}

Ptr<Stmt> Parser::gen_do_while_stmt()
{
    get_next_token();

    auto block = gen_block();

    check<TokenType::While>("expected keyword 'while' after 'do'");
    get_next_token();

    auto pos = tokenizer_.last_pos();
    auto cond = gen_expr();
    cond->pos = pos;
    return make_unique<DoWhileStmt>(move(cond), move(block));
}

Ptr<Stmt> Parser::gen_foreach_stmt()
{
    get_next_token();

    auto expression = gen_expr();
    Ptr<IdentifierExpr> id = nullptr;

    if (current_token_.type == TokenType::As)
    {
        get_next_token();
        check<TokenType::Identifier>("expected an identifier here");
        id = gen_ident_expr();
    }

    auto block = gen_block();
    return make_unique<ForeachStmt>(
        move(expression), move(id), move(block));
}

Ptr<Stmt> Parser::gen_return_stmt()
{
    get_next_token();

    ExprList exprs;
    exprs.push_back(gen_delay_expr());
    while (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        exprs.push_back(gen_delay_expr());
    }

    return make_unique<ReturnStmt>(move(exprs));
}

Ptr<Expr> Parser::gen_delay_expr()
{
    try_continue();
    if (current_token_.type == TokenType::Delay)
    {
        get_next_token();
        return make_unique<DelayExpr>(gen_expr());
    }
    else
    {
        return gen_expr();
    }
}

Ptr<Expr> Parser::gen_expr(int layer)
{
    if (layer == -1)
    {
        auto expr = gen_expr(0);
        if (current_token_.type == TokenType::Ques)
        {
            auto pos = tokenizer_.last_pos();
            get_next_token();
            auto true_expr = gen_expr();
            eat<TokenType::Comma>("expected ',' here");
            auto false_expr = gen_expr();
            expr = make_unique<QuesExpr>(
                move(expr), move(true_expr), move(false_expr)
            );
            expr->pos = pos;
        }
        return expr;
    }

    // parse unary operation or term expression
    if (Size(layer) == operators::bop_priorities.size())
    {
        if (operators::unary_ops.count(current_token_.type))
        {
            auto pos = tokenizer_.last_pos();
            auto op = current_token_;
            get_next_token();
            try_continue();
            auto expr = make_unique<UnaryOperatorExpr>(op, gen_expr(layer));
            expr->pos = pos;
            return expr;
        }
        else
        {
            return gen_term_tail(gen_term());
        }
    }

    auto lhs = gen_expr(layer + 1);
    auto op = current_token_;
    // user `if` if right-associative
    while (operators::bops_at_layer(Size(layer)).count(op.type))
    {
        auto pos = tokenizer_.last_pos();
        get_next_token();
        // gen_expr(layer) if right-associative
        auto rhs = gen_expr(layer + 1);
        if (lhs->is_integer_expr()
            && rhs->is_integer_expr())
        {
            auto alias = reinterpret_cast<IntegerExpr *>(lhs.get());
            auto rv = reinterpret_cast<IntegerExpr *>(rhs.get())->value;
            switch (op.type)
            {
            case TokenType::Add:
                alias->value += rv;
                break;

            case TokenType::Sub:
                alias->value -= rv;
                break;

            case TokenType::Mul:
                alias->value *= rv;
                break;

            case TokenType::Div:
                alias->value /= rv;
                break;

            case TokenType::Mod:
                alias->value %= rv;
                break;

            case TokenType::And:
                lhs = make_unique<BoolExpr>(alias->value && rv);
                break;

            case TokenType::Or:
                lhs = make_unique<BoolExpr>(alias->value || rv);
                break;

            case TokenType::Is:
                lhs = make_unique<BoolExpr>(alias->value == rv);
                break;

            case TokenType::CEQ:
                lhs = make_unique<BoolExpr>(alias->value == rv);
                break;

            case TokenType::CNE:
                lhs = make_unique<BoolExpr>(alias->value != rv);
                break;

            case TokenType::CLT:
                lhs = make_unique<BoolExpr>(alias->value < rv);
                break;

            case TokenType::CLE:
                lhs = make_unique<BoolExpr>(alias->value <= rv);
                break;

            case TokenType::CGT:
                lhs = make_unique<BoolExpr>(alias->value > rv);
                break;

            case TokenType::CGE:
                lhs = make_unique<BoolExpr>(alias->value >= rv);
                break;

            default:
                lhs = make_unique<BinaryOperatorExpr>(move(lhs), op, move(rhs));
                break;
            }
        }
        else
        {
            lhs = make_unique<BinaryOperatorExpr>(move(lhs), op, move(rhs));
            lhs->pos = pos;
        }
        // delete if right-associative
        op = current_token_;
    }
    return lhs;
}

Ptr<Expr> Parser::gen_term()
{
    try_continue();
    Ptr<Expr> node = nullptr;
    switch (current_token_.type)
    {
    case TokenType::Identifier:
        return gen_ident_expr();

    case TokenType::Integer:
    case TokenType::Double:
        return gen_numeric_expr();

    case TokenType::None:
        return gen_none_expr();

    case TokenType::True:
    case TokenType::False:
        return gen_boolean_expr();

    case TokenType::String:
        return gen_string_expr();

    case TokenType::LParen:
        get_next_token();
        node = gen_expr();
        eat<TokenType::RParen>("expected ')' here");
        return node;

    case TokenType::At:
        get_next_token();
        return gen_lambda_expr();

    case TokenType::Match:
        return gen_match_expr();

    case TokenType::LBracket:
        return gen_list_expr();

    case TokenType::Enum:
        return gen_enum_expr();

    case TokenType::Dict:
        return gen_dict_expr();

    case TokenType::Class:
        return gen_class_expr();

    case TokenType::LBrace:
        return make_unique<LambdaExpr>(ParameterList{}, gen_block());

    default:
        throw parse_error("expected an expr here");
    }
}

Ptr<Expr> Parser::gen_term_tail(Ptr<Expr> expr)
{
    try_continue();
    for (bool cond = true; cond;)
    {
        try_continue();
        switch (current_token_.type)
        {
        case TokenType::Dot:
            expr = gen_dot_expr(move(expr));
            break;

        case TokenType::LParen:
        {
            auto pos = tokenizer_.last_pos();
            expr = make_unique<ParenOperatorExpr>(move(expr), gen_arguments());
            expr->pos = pos;
        }
            break;

        case TokenType::LBracket:
        {
            expr = gen_index_expr(move(expr));
        }
            break;

        default:
            cond = false;
            break;
        }
    }

    if (current_token_.type == TokenType::Colon)
    {
        auto op = current_token_;
        get_next_token();
        return make_unique<BinaryOperatorExpr>(
            move(expr), op, gen_delay_expr()
        );
    }

    return expr;
}

string Parser::gen_ident_rawstr()
{
    check<TokenType::Identifier>("expect an identifier here");
    auto result = current_token_.value;
    get_next_token();
    return result;
}

Ptr<IdentifierExpr> Parser::gen_ident_expr()
{
    return make_unique<IdentifierExpr>(gen_ident_rawstr());
}

Ptr<Expr> Parser::gen_numeric_expr()
{
    Ptr<Expr> numeric_expr = nullptr;
    if (current_token_.type == TokenType::Integer)
    {
        numeric_expr = make_unique<IntegerExpr>(stoll(current_token_.value));
    }
    else
    {
        numeric_expr = make_unique<FloatExpr>(stod(current_token_.value));
    }
    get_next_token();
    return numeric_expr;
}

Ptr<Expr> Parser::gen_none_expr()
{
    get_next_token();
    return make_unique<NoneExpr>();
}

Ptr<Expr> Parser::gen_boolean_expr()
{
    auto bool_expr = make_unique<BoolExpr>(
        current_token_.type == TokenType::True
        ? true : false
    );
    get_next_token();
    return bool_expr;
}

Ptr<Expr> Parser::gen_string_expr()
{
    auto string_expr = make_unique<StringExpr>(current_token_.value);
    get_next_token();
    return string_expr;
}

Ptr<Expr> Parser::gen_dot_expr(Ptr<Expr> left)
{
    auto pos = tokenizer_.last_pos();
    get_next_token();
    auto dot_expr = make_unique<DotExpr>(move(left), gen_ident_expr());
    dot_expr->pos = pos;
    return dot_expr;
}

Ptr<Expr> Parser::gen_enum_expr()
{
    get_next_token();
    auto enum_expr = make_unique<EnumExpr>();
    eat<TokenType::LBrace>("expected '{' here");

    int64_t base = 0;
    while (current_token_.type != TokenType::RBrace)
    {
        auto ident = gen_ident_expr();
        if (current_token_.type == TokenType::Colon)
        {
            get_next_token();
            check<TokenType::Integer>("expected integer here");
            base = stoll(current_token_.value);
            get_next_token();
        }
        enum_expr->decls.push_back(
            make_unique<VariableDeclarationStmt>(move(ident),
                make_unique<IntegerExpr>(base++), true
            )
        );

        if (current_token_.type == TokenType::Comma)
        {
            get_next_token();
        }
        else
        {
            check<TokenType::RBrace>("expected '}'");
        }
    }
    get_next_token();

    return enum_expr;
}

Ptr<Expr> Parser::gen_dict_expr()
{
    get_next_token(); // eat `dict`
    eat<TokenType::LBrace>("expected '{'");
    auto dict_expr = make_unique<DictExpr>();

    while (current_token_.type != TokenType::RBrace)
    {
        dict_expr->keys.push_back(gen_expr());
        eat<TokenType::Ret>("expected '=>'");

        dict_expr->values.push_back(gen_expr());
        if (current_token_.type == TokenType::Comma)
        {
            get_next_token();
        }
        else
        {
            check<TokenType::RBrace>("expected '}'");
        }
    }
    get_next_token();
    return dict_expr;
}

Ptr<Expr> Parser::gen_class_expr()
{
    get_next_token(); // eat 'class'

    /**
     * TODO: COMPLETE GEN_CLASS_EXPR
    */

    return nullptr;
}

/**
 * generate lambda expr like:
 *   @(): expr
 *   @(), stmt
 *   @() {}
 *
 * '()' cannot be ignored because anole enable '@var: expr and @var()...',
 *   so '()' is viewed as the component of lambda
*/
Ptr<Expr> Parser::gen_lambda_expr()
{
    // eat '('
    get_next_token();
    auto parameters = gen_parameters();
    // eat ')'
    get_next_token();

    try_continue();
    Ptr<BlockExpr> block = nullptr;

    if (current_token_.type == TokenType::Colon)
    {
        get_next_token();
        block = make_unique<BlockExpr>();
        ExprList exprs;
        exprs.push_back(gen_delay_expr());
        while (current_token_.type == TokenType::Comma)
        {
            get_next_token();
            exprs.push_back(gen_delay_expr());
        }

        block->statements.push_back(make_unique<ReturnStmt>(move(exprs)));
    }
    else
    {
        block = gen_block();
    }

    return gen_term_tail(make_unique<LambdaExpr>(move(parameters), move(block)));
}

/**
 * generate match expr as the following:
 *  match value {
 *      1, 2, 3, 4 => "smaller than five",
 *      5 => "five"
 *  } else "bigger than five"
*/
Ptr<Expr> Parser::gen_match_expr()
{
    auto match_expr = make_unique<MatchExpr>();

    get_next_token(); // eat 'match'
    match_expr->expr = gen_expr();

    eat<TokenType::LBrace>("expected '{'");

    while (current_token_.type != TokenType::RBrace)
    {
        match_expr->keylists.push_back({});
        match_expr->keylists.back().push_back(gen_expr());
        match_expr->keylists.back().back()->pos = tokenizer_.last_pos();
        while (current_token_.type == TokenType::Comma)
        {
            try_continue();
            get_next_token();
            match_expr->keylists.back().push_back(gen_expr());
            match_expr->keylists.back().back()->pos = tokenizer_.last_pos();
        }

        eat<TokenType::Ret>("expected symbol '=>'");

        match_expr->values.push_back(gen_expr());

        if (current_token_.type == TokenType::Comma)
        {
            get_next_token();
        }
        else
        {
            check<TokenType::RBrace>("expected '}'");
        }

        try_continue();
    }

    get_next_token(); // eat '}'

    // check 'else' after 'match {}'
    if (current_token_.type == TokenType::Else)
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
    eat<TokenType::LBracket>(); // eat '['
    auto list_expr = make_unique<ListExpr>();
    while (current_token_.type != TokenType::RBracket)
    {
        list_expr->exprs.push_back(gen_expr());
        if (current_token_.type == TokenType::Comma)
        {
            get_next_token();
        }
    }
    get_next_token(); // eat(']')
    return list_expr;
}
}
