#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "Object.h"

class Env;

class Stmt;
class Expr;

typedef std::vector<Stmt *> StatementList;
typedef std::vector<Expr *> ExpressionList;

class Node
{
public:
    virtual ~Node(){}
    virtual Object* runCode(Env *) = 0;
};

class Stmt : public Node {};
class Expr : public Node {};

class BlockExpr : public Expr
{
public:
    StatementList statements;
    BlockExpr() {}
    virtual Object *runCode(Env *);
};

class IntegerExpr : public Expr
{
public:
    long long value;
    IntegerExpr(long long value): value(value) {}
    virtual Object *runCode(Env *);
};

class IdentifierExpr : public Expr
{
public:
    std::string name;
    IdentifierExpr(std::string &name): name(name) {}
    virtual Object *runCode(Env *);
};

class ExprStmt : public Stmt
{
public:
    Expr *assignment;
    ExprStmt(Expr *assignment): assignment(assignment) {}
    virtual Object *runCode(Env *);
};

class VariableDeclarationStmt : public Stmt
{
public:
    IdentifierExpr &id;
    Expr *assignment;
    VariableDeclarationStmt(IdentifierExpr &id, Expr *assignment): id(id), assignment(assignment) {}
    virtual Object *runCode(Env *);
};

#endif //__NODE_H__