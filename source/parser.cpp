#include <set>
#include <exception>
#include <functional>
#include "node.hpp"
#include "parser.hpp"

#define THROW(MESSAGE)                      throw runtime_error(MESSAGE)
#define CHECK_AND_THROW(TOKEN_ID, MESSAGE)  if (currentToken.tokenId != TOKEN_ID) \
                                                THROW(MESSAGE)

using namespace std;

namespace ice_language
{
Parser::Parser(istream &in)
  : tokenizer(in)
{
    getNextToken();
}

// use when interacting & return stmt node
shared_ptr<Node> Parser::getNode()
{
    return genStmt();
}

// update iToken when cannot find the next token
Token Parser::getNextToken()
{
    return (currentToken = tokenizer.next());
}

ExprList Parser::genArguments()
{
    ExprList args;
    getNextToken(); // eat '('
    while (currentToken.tokenId != TOKEN::TRPAREN)
    {
        args.push_back(genExpr());
        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken(); // eat ','
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRPAREN, "miss ')'");
        }
    }
    getNextToken(); // eat ')'
    return args;
}

VarDeclList Parser::genDeclArguments()
{
    VarDeclList args;
    getNextToken(); // eat '('
    while (currentToken.tokenId != TOKEN::TRPAREN)
    {
        auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
        EXPR expression = make_shared<NoneExpr>();
        if (currentToken.tokenId == TOKEN::TASSIGN)
        {
            getNextToken();
            expression = genExpr();
        }
        args.push_back(make_shared<VariableDeclarationStmt>(id, expression));
        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRPAREN, "miss ')'");
        }
    }
    getNextToken(); // eat ')'
    return args;
}

// usually use when interacting
BLOCK_EXPR Parser::genStmts()
{
    auto stmts = make_shared<BlockExpr>();
    while (currentToken.tokenId != TOKEN::TEND)
    {
        stmts->statements.push_back(genStmt());
    }
    return stmts;
}

// gen normal block as {...}
BLOCK_EXPR Parser::genBlock()
{
    CHECK_AND_THROW(TOKEN::TLBRACE, "missing symbol '{'");
    getNextToken(); // eat '{'

    auto block = make_shared<BlockExpr>();
    // '}' means the end of a block
    while (currentToken.tokenId != TOKEN::TRBRACE)
    {
        getNextToken();
        auto stmt = genStmt();
        if (stmt) block->statements.push_back(stmt);
    }
    getNextToken(); // eat '}'

    return block;
}

EXPR Parser::genIndexExpr(EXPR expression)
{
    getNextToken();

    auto node = genExpr();

    CHECK_AND_THROW(TOKEN::TRBRACKET, "missing symbol ']'");
    getNextToken();

    return make_shared<IndexExpr>(expression, node);
}

// generate normal statement
STMT Parser::genStmt()
{
    switch (currentToken.tokenId)
    {
    // @ is special
    case TOKEN::TAT:
        if (getNextToken().tokenId == TOKEN::TLPAREN)
        {
            return make_shared<ExprStmt>(genLambdaExpr());
        }
        else if (currentToken.tokenId != TOKEN::TEND)
        {
            return genDeclOrAssign();
        }
        break;

    case TOKEN::TATAT:
        return genClassDecl();

    case TOKEN::TUSING:
        return genUsingStmt();

    case TOKEN::TIF:
        return genIfElse();

    case TOKEN::TWHILE:
        return genWhileStmt();

    case TOKEN::TDO:
        return genDoWhileStmt();

    case TOKEN::TFOR:
        return genForStmt();

    case TOKEN::TFOREACH:
        return genForeachStmt();

    case TOKEN::TBREAK:
        getNextToken();
        return make_shared<BreakStmt>();

    case TOKEN::TCONTINUE:
        getNextToken();
        return make_shared<ContinueStmt>();

    case TOKEN::TRETURN:
        return genReturnStmt();

    case TOKEN::TIDENTIFIER:
    case TOKEN::TSUB:
    case TOKEN::TINTEGER:
    case TOKEN::TDOUBLE:
    case TOKEN::TNONE:
    case TOKEN::TTRUE:
    case TOKEN::TFALSE:
    case TOKEN::TSTRING:
    case TOKEN::TLPAREN:
    case TOKEN::TNEW:
    case TOKEN::TMATCH:
    case TOKEN::TLBRACKET:
    case TOKEN::TLBRACE:
    case TOKEN::TNOT:
        return make_shared<ExprStmt>(genExpr());

    default:
        break;
    }
    return nullptr;
}

// generate declaration or assignment (@.var:)
STMT Parser::genDeclOrAssign()
{
    if (currentToken.tokenId == TOKEN::TDOT)
    {
        return genVarAssign();
    }

    EXPR node = genIdent();

    while (currentToken.tokenId == TOKEN::TLPAREN
      || currentToken.tokenId == TOKEN::TDOT
      || currentToken.tokenId == TOKEN::TLBRACKET)
    {
        if (currentToken.tokenId == TOKEN::TLPAREN)
        {
            node = make_shared<ParenOperatorExpr>(node,
                genArguments());
        }
        else if (currentToken.tokenId == TOKEN::TDOT)
        {
            node = genDotExpr(node);
        }
        else if (currentToken.tokenId == TOKEN::TLBRACKET)
        {
            node = genIndexExpr(node);
        }
    }

    getNextToken();
    return make_shared<VariableDeclarationStmt>(node, genExpr());
}

// generate assignment as @.ident: expr
STMT Parser::genVarAssign()
{
    getNextToken();

    auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());

    CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
    getNextToken();

    return make_shared<VariableAssignStmt>(id, genExpr());
}

STMT Parser::genClassDecl()
{
    getNextToken();

    auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());

    IdentList bases;
    CHECK_AND_THROW(TOKEN::TLPAREN, "missing symbol '('");
    getNextToken();

    while (currentToken.tokenId != TOKEN::TRPAREN)
    {
        bases.push_back(dynamic_pointer_cast<IdentifierExpr>(genIdent()));
        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRPAREN, "miss ')'");
        }
    }
    getNextToken();

    auto block = genBlock();

    return make_shared<ClassDeclarationStmt>(id, bases, block);
}

STMT Parser::genUsingStmt()
{
    getNextToken();
    CHECK_AND_THROW(TOKEN::TIDENTIFIER, "missing an identifier after 'using'");
    auto usingStmt = make_shared<UsingStmt>(currentToken.value);
    getNextToken();
    return usingStmt;
}

STMT Parser::genIfElse()
{
    getNextToken();
    auto cond = genExpr();
    auto blockTrue = genBlock();
    auto elseStmt = dynamic_pointer_cast<IfElseStmt>(genIfElseTail());
    return make_shared<IfElseStmt>(cond, blockTrue, elseStmt);
}

STMT Parser::genIfElseTail()
{
    getNextToken();

    if (currentToken.tokenId == TOKEN::TELIF)
    {
        return genIfElse();
    }
    else if (currentToken.tokenId == TOKEN::TELSE)
    {
        getNextToken();
        auto block = genBlock();
        return make_shared<IfElseStmt>(make_shared<BoolExpr>(true), block, nullptr);
    }
    else return nullptr;
}

STMT Parser::genWhileStmt()
{
    getNextToken();

    auto cond = genExpr();
    auto block = genBlock();
    return make_shared<WhileStmt>(cond, block);
}

STMT Parser::genDoWhileStmt()
{
    getNextToken();

    auto block = genBlock();

    CHECK_AND_THROW(TOKEN::TWHILE, "missing keyword 'while' after 'do'");
    getNextToken();

    auto cond = genExpr();
    return make_shared<DoWhileStmt>(cond, block);
}

STMT Parser::genForStmt()
{
    getNextToken();

    auto begin = genExpr();

    CHECK_AND_THROW(TOKEN::TTO, "missing keyword 'to' in for");
    getNextToken();

    auto end = genExpr();
    shared_ptr<IdentifierExpr> id = nullptr;
    if (currentToken.tokenId == TOKEN::TAS)
    {
        getNextToken();
        id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
    }
    auto block = genBlock();

    return make_shared<ForStmt>(begin, end, id, block);
}

STMT Parser::genForeachStmt()
{
    getNextToken();

    auto expression = genExpr();

    CHECK_AND_THROW(TOKEN::TAS, "missing keyword 'as' in foreach");
    getNextToken();

    auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
    auto block = genBlock();

    return make_shared<ForeachStmt>(expression, id, block);
}

STMT Parser::genReturnStmt()
{
    getNextToken();
    return make_shared<ReturnStmt>(genExpr());
}

static const vector<set<TOKEN>> operators
{
    { TOKEN::TOR },
    { TOKEN::TAND },
    { TOKEN::TCEQ, TOKEN::TCNE, TOKEN::TCLT, TOKEN::TCLE, TOKEN::TCGT, TOKEN::TCGE },
    { TOKEN::TADD, TOKEN::TSUB },
    { TOKEN::TMUL, TOKEN::TDIV, TOKEN::TMOD },
    { TOKEN::TNOT, TOKEN::TSUB }
};

EXPR Parser::genExpr(int priority)
{
    if (priority == operators.size() - 1)
    {
        if (operators[priority].count(currentToken.tokenId))
        {
            auto op = currentToken.tokenId;
            getNextToken();
            return make_shared<UnaryOperatorExpr>(
                op, genExpr(priority)
            );
        }
        else
        {
            return genTermTail(genTerm());
        }
    }

    auto lhs = genExpr(priority + 1);
    auto op = currentToken.tokenId;
    while (operators[priority].count(op))
    {
        getNextToken();
        auto rhs = genExpr(priority + 1);
        if (rhs == nullptr)
        {
            return nullptr;
        }
        lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
        op = currentToken.tokenId;
    }
    return lhs;
}

EXPR Parser::genTerm()
{
    EXPR node = nullptr;
    switch (currentToken.tokenId)
    {
    case TOKEN::TIDENTIFIER:
        return genIdent();

    case TOKEN::TINTEGER:
    case TOKEN::TDOUBLE:
        return genNumeric();

    case TOKEN::TNONE:
        return genNone();

    case TOKEN::TTRUE:
    case TOKEN::TFALSE:
        return genBoolean();

    case TOKEN::TSTRING:
        return genString();

    case TOKEN::TLPAREN:
        getNextToken();
        node = genExpr();
        CHECK_AND_THROW(TOKEN::TRPAREN, "missing symbol ')' .");
        getNextToken();
        return node;

    case TOKEN::TAT:
        getNextToken();
        return genLambdaExpr();

    case TOKEN::TNEW:
        return genNewExpr();

    case TOKEN::TMATCH:
        return genMatchExpr();

    case TOKEN::TLBRACKET:
        return genListExpr();

    case TOKEN::TLBRACE:
        return genEnumOrDict();

    default:
        return nullptr;
    }
}

EXPR Parser::genTermTail(EXPR expr)
{
    while (currentToken.tokenId == TOKEN::TDOT
      || currentToken.tokenId == TOKEN::TLPAREN
      || currentToken.tokenId == TOKEN::TLBRACKET)
    {
        if (currentToken.tokenId == TOKEN::TDOT)
        {
            expr = genDotExpr(expr);
        }
        else if (currentToken.tokenId == TOKEN::TLPAREN)
        {
            expr = make_shared<ParenOperatorExpr>(expr, genArguments());
        }
        else // if Token is TLBRACKET
        {
            expr = genIndexExpr(expr);
        }
    }
    return expr;
}

EXPR Parser::genIdent()
{
    auto identExpr = make_shared<IdentifierExpr>(currentToken.value);
    getNextToken();
    return identExpr;
}

EXPR Parser::genNumeric()
{
    EXPR numericExpr = nullptr;
    if (currentToken.tokenId == TOKEN::TINTEGER)
    {
        numericExpr = make_shared<IntegerExpr>(stoi(currentToken.value));
    }
    else
    {
        numericExpr = make_shared<FloatExpr>(stod(currentToken.value));
    }
    getNextToken();
    return numericExpr;
}

EXPR Parser::genNone()
{
    getNextToken();
    return make_shared<NoneExpr>();
}

EXPR Parser::genBoolean()
{
    auto boolExpr = make_shared<BoolExpr>(
        ((currentToken.tokenId == TOKEN::TTRUE)
          ? true : false));
    getNextToken();
    return boolExpr;
}

EXPR Parser::genString()
{
    auto stringExpr = make_shared<StringExpr>(currentToken.value);
    getNextToken();
    return stringExpr;
}

EXPR Parser::genDotExpr(EXPR left)
{
    getNextToken();
    return make_shared<DotExpr>(left, genIdent());
}

// generate enum as { NAME1, NAME2, ..., NAMEN }
// or dict as {KEY1: VAL1, KEY2: VAL2, ..., KEYN: VALN}
EXPR Parser::genEnumOrDict()
{
    getNextToken();

    if (currentToken.tokenId == TOKEN::TRBRACE)
    {
        getNextToken();
        return make_shared<DictExpr>();
    }
    auto node = genExpr();
    if (currentToken.tokenId == TOKEN::TASSIGN)
    {
        node = genDictExpr(node);
    }
    else
    {
        node = genEnumExpr(node);
    }
    CHECK_AND_THROW(TOKEN::TRBRACE, "missing symbol '}'");
    getNextToken();
    return node;
}

EXPR Parser::genEnumExpr(EXPR first)
{
    IdentList enumerators;
    enumerators.push_back(dynamic_pointer_cast<IdentifierExpr>(first));
    if (currentToken.tokenId == TOKEN::TRBRACE)
    {
        return make_shared<EnumExpr>(enumerators);
    }
    getNextToken();
    while (currentToken.tokenId == TOKEN::TIDENTIFIER)
    {
        enumerators.push_back(dynamic_pointer_cast<IdentifierExpr>(genIdent()));
        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
            CHECK_AND_THROW(TOKEN::TIDENTIFIER, "miss identifier");
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRBRACE, "miss '}'");
        }
    }
    return make_shared<EnumExpr>(enumerators);
}

EXPR Parser::genDictExpr(EXPR first)
{
    CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
    ExprList keys, values;
    keys.push_back(first);

    getNextToken();

    values.push_back(genExpr());
    if (currentToken.tokenId == TOKEN::TCOMMA) getNextToken();
    while (currentToken.tokenId != TOKEN::TRBRACE)
    {
        keys.push_back(genExpr());

        CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
        getNextToken();

        values.push_back(genExpr());
        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRBRACE, "miss '}'");
        }
    }
    return make_shared<DictExpr>(keys, values);
}

// generate lambda expr as @(): expr, @(){} and also @(){}()..
EXPR Parser::genLambdaExpr()
{
    auto args = genDeclArguments();
    BLOCK_EXPR block = nullptr;

    if (currentToken.tokenId == TOKEN::TASSIGN)
    {
        getNextToken();
        block = make_shared<BlockExpr>();
        block->statements.push_back(make_shared<ReturnStmt>(genExpr()));
    }
    else if (currentToken.tokenId == TOKEN::TLBRACE)
    {
        block = genBlock();
    }
    else
    {
        THROW("missing symbol ':' or '{' after @()");
    }

    EXPR node = make_shared<LambdaExpr>(args, block);
    while (currentToken.tokenId == TOKEN::TLPAREN)
    {
        node = make_shared<ParenOperatorExpr>(node, genArguments());
    }

    return node;
}

// generate new expr as @instance: new Class()
EXPR Parser::genNewExpr()
{
    getNextToken();

    auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
    CHECK_AND_THROW(TOKEN::TLPAREN, "missing symbol '('");
    ExprList args = genArguments();

    return make_shared<NewExpr>(id, args);
}

/* EXPR SyntaxAnalyzer::genMatchExpr()
generate match expr as follow:
    match value {
        1, 2, 3, 4 => "smaller than five",
        5 => "five"
    } else "bigger than five"
*/
EXPR Parser::genMatchExpr()
{
    getNextToken(); // eat 'match'
    auto expression = genExpr();
    ExprList mat_expressions, ret_expressions;

    CHECK_AND_THROW(TOKEN::TLBRACE, "missing symbol '{'");
    getNextToken(); // eat '{'

    while (currentToken.tokenId != TOKEN::TRBRACE)
    {
        int counter = 1;
        mat_expressions.push_back(genExpr());

        while (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
            mat_expressions.push_back(genExpr());
            counter++;
        }

        CHECK_AND_THROW(TOKEN::TRET, "missing symbol '=>'");
        getNextToken();

        auto ret_expression = genExpr();
        while (counter--)
        {
            ret_expressions.push_back(ret_expression);
        }

        if (currentToken.tokenId == TOKEN::TCOMMA)
        {
            getNextToken();
        }
        else
        {
            CHECK_AND_THROW(TOKEN::TRBRACE, "miss '}'");
        }
    }

    getNextToken(); // eat '}'

    // check 'else' after 'match {}'
    if (currentToken.tokenId == TOKEN::TELSE)
    {
        getNextToken();
        return make_shared<MatchExpr>(expression,
            mat_expressions, ret_expressions, genExpr());
    }
    return make_shared<MatchExpr>(expression,
        mat_expressions, ret_expressions, make_shared<NoneExpr>());
}

// generate list as [expr1, expr2, ..., exprN]
EXPR Parser::genListExpr()
{
    getNextToken(); // eat '['

    ExprList expressions;
    while (currentToken.tokenId != TOKEN::TRBRACKET)
    {
        expressions.push_back(genExpr());
        if (currentToken.tokenId == TOKEN::TCOMMA) getNextToken();
    }

    getNextToken(); // eat(']')

    return make_shared<ListExpr>(expressions);
}
}

#undef THROW
#undef CHECK_AND_THROW
