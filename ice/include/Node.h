#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "IceObject.h"
#include "Token.h"

class Env;

class Stmt;
class Expr;

typedef std::vector<Stmt *> StatementList;
typedef std::vector<Expr *> ExpressionList;

class Node
{
  public:
    virtual ~Node() {}
    virtual IceObject *runCode(Env *) = 0;
};

class Stmt : public Node
{
};
class Expr : public Node
{
};

class BlockExpr : public Expr
{
  public:
    StatementList statements;
    BlockExpr() {}
    virtual IceObject *runCode(Env *);
};

class IntegerExpr : public Expr
{
  public:
    long value;
    IntegerExpr(long value) : value(value) {}
    virtual IceObject *runCode(Env *);
};

class DoubleExpr : public Expr
{
  public:
    double value;
    DoubleExpr(double value) : value(value) {}
    virtual IceObject *runCode(Env *);
};

class IdentifierExpr : public Expr
{
  public:
    std::string name;
    IdentifierExpr(std::string &name) : name(name) {}
    virtual IceObject *runCode(Env *);
};

class BinaryOperatorExpr : public Expr
{
  public:
    Expr *lhs;
    Token::TOKEN op;
    Expr *rhs;
    BinaryOperatorExpr(Expr *lhs, Token::TOKEN op, Expr *rhs) : lhs(lhs), op(op), rhs(rhs) {}
    virtual IceObject *runCode(Env *);
};

class ExprStmt : public Stmt
{
  public:
    Expr *assignment;
    ExprStmt(Expr *assignment) : assignment(assignment) {}
    virtual IceObject *runCode(Env *);
};

class VariableDeclarationStmt : public Stmt
{
  public:
    IdentifierExpr *id;
    Expr *assignment;
    VariableDeclarationStmt(IdentifierExpr *id, Expr *assignment) : id(id), assignment(assignment) {}
    virtual IceObject *runCode(Env *);
};

#endif //__NODE_H__