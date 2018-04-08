#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

class Env;

class Stmt;
class Expr;

typedef std::vector<Stmt *> StatementList;
typedef std::vector<Expr *> ExpressionList;

class Node
{
public:
    virtual ~Node(){}
    virtual void* runCode(Env *) = 0;
};

class Stmt : public Node {};
class Expr : public Node {};

class BlockExpr : public Expr
{
public:
    StatementList statements;
    BlockExpr() {}
    virtual void* runCode(Env *);
};

class IntegerExpr : public Expr
{
public:
    long long value;
    IntegerExpr(long long value): value(value) {}
    virtual void* runCode(Env *);
};

class IdentifierExpr : public Expr
{
public:
    std::string name;
    IdentifierExpr(std::string &name): name(name) {}
    virtual void* runCode(Env *);
};

class VariableDeclarationStmt : Stmt
{
public:
    IdentifierExpr &id;
    Expr *assignment;
    VariableDeclarationStmt(IdentifierExpr &id, Expr *assignment): id(id), assignment(assignment) {}
    virtual void* runCode(Env *);
};

#endif //__NODE_H__