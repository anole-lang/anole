#include "compiler.hpp"

#include "../error.hpp"

#include <set>
#include <algorithm>

namespace anole
{
namespace operators
{
std::set<TokenType> uops
{
    TokenType::Not, TokenType::Sub, TokenType::BNeg
};

std::vector<Size> bop_precedences
{
    100, 110, 120, 130, 140, 150, 160, 170, 180, 190
};
std::map<Size, std::set<TokenType>> bop_mapping
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

std::set<TokenType> &bops_at_precedence(Size precedence)
{
    return bop_mapping[precedence];
}

std::set<TokenType> &bops_at_layer(Size layer)
{
    return bop_mapping[bop_precedences[layer]];
}
}

void Parser::add_prefixop(const String &str)
{
    auto type = Token::add_token_type(str);
    if (type <= TokenType::End)
    {
        throw RuntimeError("can't define predefined keywords or operators");
    }

    operators::uops.insert(type);
}

void Parser::add_infixop(const String &str, Size precedence)
{
    auto type = Token::add_token_type(str);
    if (type <= TokenType::End)
    {
        throw RuntimeError("can't define predefined keywords or operators");
    }

    auto lower = lower_bound(operators::bop_precedences.begin(),
        operators::bop_precedences.end(), precedence
    );
    if (lower == operators::bop_precedences.end() || *lower != precedence)
    {
        operators::bop_precedences.insert(lower, precedence);
    }

    operators::bops_at_precedence(precedence).insert(type);
}

Parser::Parser() noexcept
  : Parser(std::cin, "<stdin>")
{
    // ...
}

Parser::Parser(std::istream &input, String name_of_input) noexcept
  : tokenizer_(input, std::move(name_of_input))
  , current_token_(tokenizer_.next_token())
{
    // ...
}

void Parser::resume()
{
    tokenizer_.resume();
    get_next_token();
}

void Parser::reset()
{
    tokenizer_.reset();
    get_next_token();
}

void Parser::set_resume_action(std::function<void()> action)
{
    resume_action_ = std::move(action);
}

Ptr<Stmt> Parser::gen_statement()
{
    while (current_token_.type == TokenType::Semicolon)
    {
        get_next_token();
    }

    auto stmt = gen_stmt();

    while (current_token_.type == TokenType::Semicolon)
    {
        get_next_token();
    }

    return stmt;
}

CompileError Parser::ParseError(const String &info)
{
    return CompileError(tokenizer_.get_err_info(info));
}

void Parser::get_next_token()
{
    current_token_ = tokenizer_.next_token();
}

// update current token when cannot find the next token
Token &Parser::next_token()
{
    return current_token_ = tokenizer_.next_token();
}

void Parser::try_resume(Size times)
{
    if (!resume_action_)
    {
        return;
    }

    if (times == 0)
    {
        while (current_token_.type == TokenType::End)
        {
            resume_action_();
        }
    }
    else
    {
        while (current_token_.type == TokenType::End && times--)
        {
            resume_action_();
        }
    }
}

ExprList Parser::gen_exprs()
{
    ExprList exprs;
    exprs.push_back(gen_delay_expr());
    while (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        exprs.push_back(gen_delay_expr());
    }
    return exprs;
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
        args.push_back(std::make_pair(std::move(expr), unpack));
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

// gen normal block as {...}
Ptr<Block> Parser::gen_block()
{
    eat<TokenType::LBrace>("expect '{'");
    try_resume();

    auto block = std::make_unique<Block>();
    // '}' means the end of a block
    while (current_token_.type != TokenType::RBrace)
    {
        block->statements.push_back(gen_statement());
        try_resume();
    }
    get_next_token(); // eat '}'

    return block;
}

// generate plain statement
Ptr<Stmt> Parser::gen_stmt()
{
    switch (current_token_.type)
    {
    case TokenType::End:
        return nullptr;

    // @ is special
    case TokenType::At:
    {
        auto type = next_token().type;
        if (type == TokenType::LParen || type == TokenType::Colon || type == TokenType::LBrace)
        {
            return std::make_unique<ExprStmt>(gen_expr());
        }
        else
        {
            return gen_decl_stmt();
        }
    }

    case TokenType::Class:
        return gen_class_decl_stmt();

    case TokenType::Use:
        return gen_use_stmt();

    case TokenType::Prefixop:
        return gen_prefixop_decl_stmt();

    case TokenType::Infixop:
        return gen_infixop_decl_stmt();

    case TokenType::If:
        return gen_if_else_stmt();

    case TokenType::While:
        return gen_while_stmt();

    case TokenType::Do:
        return gen_do_while_stmt();

    case TokenType::Foreach:
        return gen_foreach_stmt();

    case TokenType::Break:
        get_next_token();
        return std::make_unique<BreakStmt>();

    case TokenType::Continue:
        get_next_token();
        return std::make_unique<ContinueStmt>();

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
    case TokenType::Match:
    case TokenType::LBracket:
    case TokenType::LBrace:
    case TokenType::Not:
        return std::make_unique<ExprStmt>(gen_expr());

    default:
        if (operators::uops.count(current_token_.type))
        {
            return std::make_unique<ExprStmt>(gen_expr());
        }
    }
    throw ParseError("wrong token here");
}

// generate declaration or assignment (@var:)
auto Parser::gen_decl_stmt() -> Ptr<DeclarationStmt>
{
    if (current_token_.type == TokenType::LBracket)
    {
        auto variables = gen_variables();
        if (current_token_.type != TokenType::Colon)
        {
            return std::make_unique<MultiVarsDeclarationStmt>(std::move(variables));
        }

        get_next_token();
        return std::make_unique<MultiVarsDeclarationStmt>(std::move(variables), gen_exprs());
    }

    bool is_ref = false;
    if (current_token_.type == TokenType::BAnd)
    {
        is_ref = true;
        get_next_token();
    }

    auto name = gen_ident_rawstr();
    switch (current_token_.type)
    {
    case TokenType::Comma:
    {
        get_next_token();
        auto vars = gen_variables();
        vars.push_front(
            std::make_unique<MultiVarsDeclarationStmt::SingleDeclVariable>(std::move(name), is_ref)
        );

        // @var1, ..., varn
        if (current_token_.type != TokenType::Colon)
        {
            return std::make_unique<MultiVarsDeclarationStmt>(std::move(vars));
        }

        // @var1, ..., varn: expr
        get_next_token();
        return std::make_unique<MultiVarsDeclarationStmt>(std::move(vars), gen_exprs());
    }

    case TokenType::Colon:
        get_next_token();
        return std::make_unique<NormalDeclarationStmt>(
            std::move(name), gen_delay_expr(), is_ref
        );

    case TokenType::LParen:
    {
        if (is_ref)
        {
            throw ParseError("& cannot be here");
        }

        get_next_token();
        auto parameters = gen_parameters();
        get_next_token();

        try_resume();

        Ptr<Block> block = nullptr;
        if (current_token_.type == TokenType::Colon)
        {
            get_next_token();
            block = std::make_unique<Block>();
            block->statements.push_back(std::make_unique<ReturnStmt>(gen_exprs()));
        }
        else
        {
            block = gen_block();
        }

        return std::make_unique<NormalDeclarationStmt>(std::move(name),
            std::make_unique<LambdaExpr>(std::move(parameters), std::move(block)), true
        );
    }

    default:
        if (is_ref)
        {
            throw ParseError("reference should be binded with other variable");
        }
        break;
    }
    return std::make_unique<NormalDeclarationStmt>(
        std::move(name), std::make_unique<NoneExpr>(), false
    );
}

auto Parser::gen_variable() -> Ptr<MultiVarsDeclarationStmt::DeclVariable>
{
    if (current_token_.type == TokenType::BAnd || current_token_.type == TokenType::Identifier)
    {
        bool is_ref = false;
        if (current_token_.type == TokenType::BAnd)
        {
            is_ref = true;
            get_next_token();
        }

        auto name = gen_ident_rawstr();
        return std::make_unique<MultiVarsDeclarationStmt::SingleDeclVariable>(std::move(name), is_ref);
    }
    else if (current_token_.type == TokenType::LBracket)
    {
        get_next_token();
        auto var = std::make_unique<MultiVarsDeclarationStmt::MultiDeclVariables>(gen_variables());
        eat<TokenType::RBracket>("expect ']' here");
        return var;
    }
    else
    {
        throw CompileError("expect an identifier");
    }
}

auto Parser::gen_variables() -> MultiVarsDeclarationStmt::DeclVariableList
{
    MultiVarsDeclarationStmt::DeclVariableList variables;

    variables.push_back(gen_variable());
    while (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        variables.push_back(gen_variable());
    }

    return variables;
}

Ptr<Stmt> Parser::gen_class_decl_stmt()
{
    auto class_expr = gen_class_expr();
    if (class_expr->name.empty())
    {
        throw ParseError("expected class name");
    }

    return std::make_unique<NormalDeclarationStmt>(
        class_expr->name, std::move(class_expr), true
    );
}

Ptr<Stmt> Parser::gen_use_stmt()
{
    UseStmt::Aliases aliases;
    UseStmt::NestedModule from;

    // eat `use`
    get_next_token();

    // use * from <module>
    if (current_token_.type == TokenType::Mul)
    {
        get_next_token();
        eat<TokenType::From>("need a module here");
        return std::make_unique<UseStmt>(std::move(aliases), gen_nested_module());
    }

    aliases.push_back(gen_alias());
    bool use_direct = aliases.back().first.back().type == UseStmt::Module::Type::Path;
    while (current_token_.type == TokenType::Comma)
    {
        get_next_token();
        aliases.push_back(gen_alias());
        use_direct |= aliases.back().first.back().type == UseStmt::Module::Type::Path;
    }

    if (current_token_.type == TokenType::From)
    {
        if (use_direct)
        {
            throw ParseError("unexpected from because there is at least one module denoted by its path");
        }
        get_next_token();
        from = gen_nested_module();
    }

    return std::make_unique<UseStmt>(std::move(aliases), std::move(from));
}

/**
 * single module may be denoted by name of path
*/
auto Parser::gen_module() -> UseStmt::Module
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
        throw ParseError("expect a module here");
    }
    // eat `<module>`
    get_next_token();
    return mod;
}

auto Parser::gen_nested_module() -> UseStmt::NestedModule
{
    UseStmt::NestedModule result;

    result.push_back(gen_module());
    while (current_token_.type == TokenType::Dot)
    {
        get_next_token();
        result.push_back(gen_module());

        if (result.back().type == UseStmt::Module::Type::Path)
        {
            throw ParseError("module denoted by path must be the top module");
        }
    }

    return result;
}

auto Parser::gen_alias() -> UseStmt::Alias
{
    UseStmt::Alias alias;
    alias.first = gen_nested_module();
    if (current_token_.type == TokenType::As)
    {
        get_next_token();
        check<TokenType::Identifier>("need the alias here");
        alias.second = current_token_.value;
        get_next_token();
    }
    else
    {
        if (alias.first.back().type == UseStmt::Module::Type::Path)
        {
            throw ParseError("use direct path of module need an alias");
        }
        else
        {
            alias.second = alias.first.back().mod;
        }
    }
    return alias;
}

Ptr<Stmt> Parser::gen_prefixop_decl_stmt()
{
    get_next_token();
    check<TokenType::Identifier>("expected an identifier here");
    return std::make_unique<PrefixopDeclarationStmt>(gen_ident_rawstr());
}

Ptr<Stmt> Parser::gen_infixop_decl_stmt()
{
    get_next_token();
    Size precedence = 50;
    if (current_token_.type == TokenType::Integer)
    {
        precedence = stoull(current_token_.value);
        get_next_token();
    }
    check<TokenType::Identifier>("expected an identifier here");
    return std::make_unique<InfixopDeclarationStmt>(gen_ident_rawstr(), precedence);
}

Ptr<Stmt> Parser::gen_if_else_stmt()
{
    eat<TokenType::If, TokenType::Elif>("expect elif");

    auto location = current_token_.location;

    auto cond = gen_expr();
    auto true_block = gen_block();
    try_resume(1);

    Ptr<AST> false_branch = nullptr;
    if (current_token_.type == TokenType::Elif)
    {
        false_branch = gen_if_else_stmt();
    }
    else if (current_token_.type == TokenType::Else)
    {
        get_next_token();
        false_branch = gen_block();
    }

    auto stmt = std::make_unique<IfElseStmt>(std::move(cond),
        std::move(true_block), std::move(false_branch)
    );
    stmt->location = location;
    return stmt;
}

Ptr<Stmt> Parser::gen_while_stmt()
{
    auto location = next_token().location;

    auto cond = gen_expr();
    auto block = gen_block();
    auto stmt = std::make_unique<WhileStmt>(std::move(cond), std::move(block));

    stmt->location = location;
    return stmt;
}

Ptr<Stmt> Parser::gen_do_while_stmt()
{
    get_next_token();

    auto block = gen_block();

    check<TokenType::While>("expected keyword 'while' after 'do'");
    auto location = next_token().location;

    auto cond = gen_expr();
    auto stmt = std::make_unique<DoWhileStmt>(std::move(cond), std::move(block));

    stmt->location = location;
    return stmt;
}

Ptr<Stmt> Parser::gen_foreach_stmt()
{
    get_next_token();

    auto expression = gen_expr();

    String varname;
    if (current_token_.type == TokenType::As)
    {
        get_next_token();
        check<TokenType::Identifier>("expected an identifier here");
        varname = gen_ident_rawstr();
    }

    auto block = gen_block();
    return std::make_unique<ForeachStmt>(
        std::move(expression), std::move(varname), std::move(block)
    );
}

Ptr<Stmt> Parser::gen_return_stmt()
{
    get_next_token();
    try_resume();

    if (current_token_.type == TokenType::Semicolon)
    {
        get_next_token();
        return std::make_unique<ReturnStmt>();
    }

    return std::make_unique<ReturnStmt>(gen_exprs());
}

Ptr<Expr> Parser::gen_delay_expr()
{
    try_resume();
    if (current_token_.type == TokenType::Delay)
    {
        get_next_token();
        return std::make_unique<DelayExpr>(gen_expr());
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
            auto location = current_token_.location;
            get_next_token();
            auto true_expr = gen_expr();
            eat<TokenType::Comma>("expected ',' here");
            auto false_expr = gen_expr();
            auto ques = std::make_unique<QuesExpr>(
                std::move(expr), std::move(true_expr), std::move(false_expr)
            );
            ques->location = location;
            expr = std::move(ques);
        }
        return expr;
    }

    // parse unary operation or term expression
    if (Size(layer) == operators::bop_precedences.size())
    {
        if (operators::uops.count(current_token_.type))
        {
            auto location = current_token_.location;
            auto op = current_token_;
            get_next_token();
            try_resume();
            auto expr = std::make_unique<UnaryOperatorExpr>(op, gen_expr(layer));
            expr->location = location;
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
        auto location = current_token_.location;
        get_next_token();
        // gen_expr(layer) if right-associative
        auto rhs = gen_expr(layer + 1);
        if (lhs->is_integer_expr() && rhs->is_integer_expr())
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
                lhs = std::make_unique<BoolExpr>(alias->value && rv);
                break;

            case TokenType::Or:
                lhs = std::make_unique<BoolExpr>(alias->value || rv);
                break;

            case TokenType::Is:
                lhs = std::make_unique<BoolExpr>(alias->value == rv);
                break;

            case TokenType::CEQ:
                lhs = std::make_unique<BoolExpr>(alias->value == rv);
                break;

            case TokenType::CNE:
                lhs = std::make_unique<BoolExpr>(alias->value != rv);
                break;

            case TokenType::CLT:
                lhs = std::make_unique<BoolExpr>(alias->value < rv);
                break;

            case TokenType::CLE:
                lhs = std::make_unique<BoolExpr>(alias->value <= rv);
                break;

            case TokenType::CGT:
                lhs = std::make_unique<BoolExpr>(alias->value > rv);
                break;

            case TokenType::CGE:
                lhs = std::make_unique<BoolExpr>(alias->value >= rv);
                break;

            default:
                lhs = std::make_unique<BinaryOperatorExpr>(std::move(lhs), op, std::move(rhs));
                break;
            }
        }
        else
        {
            auto expr = std::make_unique<BinaryOperatorExpr>(std::move(lhs), op, std::move(rhs));
            expr->location = location;
            lhs = std::move(expr);
        }
        // delete if right-associative
        op = current_token_;
    }
    return lhs;
}

Ptr<Expr> Parser::gen_term()
{
    try_resume();

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
    {
        get_next_token();
        auto node = gen_expr();
        eat<TokenType::RParen>("expected ')' here");
        return node;
    }

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
        return std::make_unique<ParenOperatorExpr>(
            std::make_unique<LambdaExpr>(gen_block())
        );

    default:
        throw ParseError("expected an expr here");
    }
}

Ptr<Expr> Parser::gen_term_tail(Ptr<Expr> expr)
{
    try_resume();
    for (bool cond = true; cond;)
    {
        try_resume();
        switch (current_token_.type)
        {
        case TokenType::Dot:
            expr = gen_dot_expr(std::move(expr));
            break;

        case TokenType::LParen:
        {
            auto location = current_token_.location;
            auto paren_expr = std::make_unique<ParenOperatorExpr>(std::move(expr), gen_arguments());
            paren_expr->location = location;
            expr = std::move(paren_expr);
        }
            break;

        case TokenType::LBracket:
        {
            expr = gen_index_expr(std::move(expr));
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
        return std::make_unique<BinaryOperatorExpr>(
            std::move(expr), op, gen_delay_expr()
        );
    }

    return expr;
}

auto Parser::gen_ident_rawstr() -> String
{
    check<TokenType::Identifier>("expect an identifier here");
    auto result = current_token_.value;
    get_next_token();
    return result;
}

Ptr<Expr> Parser::gen_ident_expr()
{
    auto location = current_token_.location;
    auto expr = std::make_unique<IdentifierExpr>(gen_ident_rawstr());
    expr->location = location;
    return expr;
}

Ptr<Expr> Parser::gen_numeric_expr()
{
    Ptr<Expr> numeric_expr = nullptr;
    if (current_token_.type == TokenType::Integer)
    {
        numeric_expr = std::make_unique<IntegerExpr>(stoll(current_token_.value));
    }
    else
    {
        numeric_expr = std::make_unique<FloatExpr>(stod(current_token_.value));
    }
    get_next_token();
    return numeric_expr;
}

Ptr<Expr> Parser::gen_none_expr()
{
    get_next_token();
    return std::make_unique<NoneExpr>();
}

Ptr<Expr> Parser::gen_boolean_expr()
{
    auto bool_expr = std::make_unique<BoolExpr>(
        current_token_.type == TokenType::True
        ? true : false
    );
    get_next_token();
    return bool_expr;
}

Ptr<Expr> Parser::gen_string_expr()
{
    auto string_expr = std::make_unique<StringExpr>(current_token_.value);
    get_next_token();
    return string_expr;
}

Ptr<Expr> Parser::gen_dot_expr(Ptr<Expr> left)
{
    auto location = current_token_.location;
    get_next_token();
    auto dor_expr = std::make_unique<DotExpr>(std::move(left), gen_ident_rawstr());
    dor_expr->location = location;
    return dor_expr;
}

Ptr<Expr> Parser::gen_index_expr(Ptr<Expr> expr)
{
    auto location = current_token_.location;
    get_next_token();

    auto node = gen_expr();

    check<TokenType::RBracket>("expected ']'");
    get_next_token();

    auto index_expr = std::make_unique<IndexExpr>(std::move(expr), std::move(node));
    index_expr->location = location;
    return index_expr;
}

Ptr<Expr> Parser::gen_enum_expr()
{
    get_next_token();
    auto enum_expr = std::make_unique<EnumExpr>();
    eat<TokenType::LBrace>("expected '{' here");

    int64_t base = 0;
    while (current_token_.type != TokenType::RBrace)
    {
        auto ident = gen_ident_rawstr();
        if (current_token_.type == TokenType::Colon)
        {
            get_next_token();
            check<TokenType::Integer>("expected integer here");
            base = stoll(current_token_.value);
            get_next_token();
        }
        enum_expr->decls.emplace_back(std::move(ident),
            std::make_unique<IntegerExpr>(base++), true
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
    auto dict_expr = std::make_unique<DictExpr>();

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

/**
 * generate lambda expr like:
 *   @(): expr
 *   @() {}
 *
 * '()' cannot be ignored because anole enable '@var: expr and @var()...',
 *   so '()' is viewed as the component of lambda
*/
Ptr<Expr> Parser::gen_lambda_expr()
{
    LambdaExpr::ParameterList parameters;

    if (current_token_.type == TokenType::LParen)
    {
        // eat '('
        get_next_token();
        parameters = gen_parameters();
        // eat ')'
        get_next_token();
    }

    try_resume();
    Ptr<Block> block = nullptr;

    if (current_token_.type == TokenType::Colon)
    {
        get_next_token();

        block = std::make_unique<Block>();
        ExprList exprs;
        exprs.push_back(gen_delay_expr());
        block->statements.push_back(std::make_unique<ReturnStmt>(std::move(exprs)));
    }
    else
    {
        block = gen_block();
    }

    return std::make_unique<LambdaExpr>(
        std::move(parameters), std::move(block)
    );
}

auto Parser::gen_parameters() -> LambdaExpr::ParameterList
{
    bool need_default = false;
    bool packed = false;

    LambdaExpr::ParameterList parameters;
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

            auto decl = std::make_unique<NormalDeclarationStmt>(
                gen_ident_rawstr(), nullptr, is_ref
            );
            parameters.push_back(std::make_pair(std::move(decl), true));
        }
        else
        {
            bool is_ref = false;
            if (current_token_.type == TokenType::BAnd)
            {
                is_ref = true;
                get_next_token();
            }

            auto ident = gen_ident_rawstr();
            Ptr<NormalDeclarationStmt> decl = nullptr;
            if (current_token_.type == TokenType::Colon)
            {
                need_default = true;
                get_next_token();
                decl = std::make_unique<NormalDeclarationStmt>(
                    std::move(ident), gen_expr(), is_ref
                );
            }
            else if (need_default)
            {
                throw ParseError("parameter without default argument cannot follow parameter with default argument");
            }
            else
            {
                decl = std::make_unique<NormalDeclarationStmt>(
                    std::move(ident), nullptr, is_ref
                );
            }

            parameters.push_back(std::make_pair(std::move(decl), false));
        }

        if (current_token_.type == TokenType::Comma)
        {
            if (packed)
            {
                throw ParseError("packed parameter should be the last parameter");
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

/**
 * generate match expr as the following:
 *  match value {
 *      1, 2, 3, 4 => "smaller than five",
 *      5 => "five",
 *      => "bigger than five"
 *  }
*/
Ptr<Expr> Parser::gen_match_expr()
{
    auto match_expr = std::make_unique<MatchExpr>();

    get_next_token(); // eat 'match'
    match_expr->expr = gen_expr();

    eat<TokenType::LBrace>("expected '{'");

    while (current_token_.type != TokenType::RBrace)
    {
        if (current_token_.type == TokenType::Ret)
        {
            if (match_expr->else_expr)
            {
                throw CompileError("redefinition of else-expr of match-expr");
            }

            get_next_token();
            match_expr->else_expr = gen_expr();

            if (current_token_.type == TokenType::Comma)
            {
                get_next_token();
            }
            else
            {
                check<TokenType::RBrace>("expected '}'");
            }

            try_resume();
            continue;
        }

        match_expr->keylists.push_back({});
        auto location = current_token_.location;
        match_expr->keylists.back().push_back(gen_expr());
        match_expr->locations.push_back(location);
        while (current_token_.type == TokenType::Comma)
        {
            try_resume();
            auto location = next_token().location;
            match_expr->keylists.back().push_back(gen_expr());
            match_expr->locations.push_back(location);
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

        try_resume();
    }

    get_next_token(); // eat '}'

    return match_expr;
}

// generate std::list as [expr1, expr2, ..., exprN]
Ptr<Expr> Parser::gen_list_expr()
{
    eat<TokenType::LBracket>(); // eat '['
    auto list_expr = std::make_unique<ListExpr>();
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

Ptr<ClassExpr> Parser::gen_class_expr()
{
    get_next_token(); // eat 'class'

    ArgumentList bases;
    if (current_token_.type == TokenType::LParen)
    {
        bases = gen_arguments();
    }

    String name;
    if (current_token_.type == TokenType::Identifier)
    {
        name = gen_ident_rawstr();
    }

    ClassExpr::DeclList members;
    eat<TokenType::LBrace>("expected '{'");
    while (current_token_.type != TokenType::RBrace)
    {
        while (current_token_.type == TokenType::Semicolon)
        {
            get_next_token();
        }

        try_resume();
        if (current_token_.type == TokenType::RBrace)
        {
            break;
        }

        auto decl = gen_decl_stmt();

        /**
         * that the member's name is __init__
         *  means it is the special constructor without return-values
         *
         * this will be promised by allowing only definition with function body
        */
        if (dynamic_cast<NormalDeclarationStmt *>(decl.get()))
        {
            auto vardecl = reinterpret_cast<NormalDeclarationStmt *>(decl.get());
            if (vardecl->name == "__init__")
            {
                if (!dynamic_cast<LambdaExpr *>(vardecl->expr.get()))
                {
                    throw ParseError("__init__ must be with function body");
                }
                else
                {
                    auto &params = reinterpret_cast<LambdaExpr *>(vardecl->expr.get())->parameters;
                    auto block = reinterpret_cast<LambdaExpr *>(vardecl->expr.get())->block.get();

                    if (params.empty())
                    {
                        throw CompileError("method need at least 1 parameter");
                    }

                    ExprList retvals;
                    retvals.push_back(std::make_unique<IdentifierExpr>(
                        params.front().first->name
                    ));
                    block->statements.push_back(
                        std::make_unique<ReturnStmt>(std::move(retvals))
                    );
                }
            }
        }
        else
        {
            throw CompileError("not support multi-declaration in class");

            /**
             * TODO:
             *  solve the problem of __init__'s return
             *
            auto muldecl = reinterpret_cast<MultiVarsDeclarationStmt *>(decl.get());

            for (auto &variable : muldecl->variables)
            {
                if (variable == "__init__")
                {
                    throw CompileError("__init__ must be with function body individually ");
                }
            }
            */
        }

        members.emplace_back(std::move(decl));
    }
    get_next_token();

    return std::make_unique<ClassExpr>(std::move(name), std::move(bases), std::move(members));
}
}
