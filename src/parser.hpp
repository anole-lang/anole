#pragma once

#include <memory>
#include <vector>
#include <istream>
#include <iostream>
#include <iterator>
#include "token.hpp"
#include "tokenizer.hpp"

namespace ice_language
{
using NODE       = std::shared_ptr<struct Node>;
using EXPR       = std::shared_ptr<struct Expr>;
using STMT       = std::shared_ptr<struct Stmt>;
using BLOCK_EXPR = std::shared_ptr<struct BlockExpr>;

class Parser
{
  public:
    Parser(std::istream &in = std::cin);
    std::shared_ptr<Node> getNode();

  private:
    Token currentToken;
    Tokenizer tokenizer;
    Token getNextToken();

    ExprList genArguments();
    VarDeclList genDeclArguments();

    BLOCK_EXPR genStmts();
    BLOCK_EXPR genBlock();

    STMT genStmt();
    STMT genDeclOrAssign();
    STMT genVarAssign();
    STMT genClassDecl();
    STMT genUsingStmt();
    STMT genIfElse();
    STMT genIfElseTail();
    STMT genWhileStmt();
    STMT genDoWhileStmt();
    STMT genForStmt();
    STMT genForeachStmt();
    STMT genReturnStmt();

    EXPR genExpr(int priority = 0);
    EXPR genTerm();
    EXPR genTermTail(EXPR expr);
    EXPR genIdent();
    EXPR genNumeric();
    EXPR genNone();
    EXPR genBoolean();
    EXPR genString();
    EXPR genDotExpr(EXPR left);
    EXPR genIndexExpr(EXPR expr);
    EXPR genEnumOrDict();
    EXPR genEnumExpr(EXPR first);
    EXPR genDictExpr(EXPR first);
    EXPR genLambdaExpr();
    EXPR genNewExpr();
    EXPR genMatchExpr();
    EXPR genListExpr();
};
}
