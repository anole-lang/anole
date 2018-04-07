#include <iostream>
#include <string>
#include <cstring>

class Stmt;
class Expr;

typedef std::vector<Stmt *> StatementList;
typedef std::vector<Expr *> ExpressionList;

class Node
{
public:
    virtual ~Node(){}
};

class Stmt : public Node {};
class Expr : public Node {};

class BlockExpr : public Expr 
{
public:
    StatementList statements;
    BlockExpr() {}
};

class IntegerExpr : public Expr
{
public:
    long long value;
    IntegerExpr(long long value): value(value) {}
};

class IdentifierExpr : public Expr
{
public:
    std::string name;
    IdentifierExpr(std::string &name): name(name) {}
};

class VariableDeclarationStmt : Stmt
{
public:
    IdentifierExpr &id;
    Expr *assignment;
    VariableDeclarationStmt(IdentifierExpr &id, Expr *assignment): id(id), assignment(assignment) {}
};