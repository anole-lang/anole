#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class StmtAST;
class ExprAST;
class IdentifierExprAST;

typedef std::vector<StmtAST *> StatementList;
typedef std::vector<ExprAST *> ExpressionList;
typedef std::vector<IdentifierExprAST *> VariableList;

class Node
{
public:
	virtual ~Node() {}
	virtual llvm::Value *codeGen(CodeGenContext &context) {}
};

class StmtAST : public Node {};

class ExprAST : public Node
{
public:
	std::string _type;
	std::string name;
};

class IntegerExprAST : public ExprAST
{
public:
	long long value;
	IntegerExprAST(long long value) : value(value) { _type = "int"; }
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class DoubleExprAST : public ExprAST
{
public:
	double value;
	DoubleExprAST(double value) : value(value) { _type = "double"; }
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class IdentifierExprAST : public ExprAST
{
public:
	IdentifierExprAST(std::string &name) { this->name = name; }
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class MethodCallExprAST : public ExprAST
{
public:
	ExpressionList arguments;
	MethodCallExprAST(std::string &name, ExpressionList &arguments) : arguments(arguments) { this->name = name; }
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class BinaryOperatorExprAST : public ExprAST
{
public:
	int op;
	ExprAST &lhs;
	ExprAST &rhs;
	BinaryOperatorExprAST(ExprAST &lhs, int op, ExprAST &rhs) : lhs(lhs), op(op), rhs(rhs) {}
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class BlockExprAST : public ExprAST
{
public:
	StatementList statements;
	BlockExprAST() {}
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class ExprStmtAST : public StmtAST
{
public:
	ExprAST &expression;
	ExprStmtAST(ExprAST &expression) : expression(expression) {}
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class VariableDeclarationStmtAST : public StmtAST
{
public:
	IdentifierExprAST &id;
	ExprAST *assignmentExpr;
	VariableDeclarationStmtAST(IdentifierExprAST &id, ExprAST *assignmentExpr) : id(id), assignmentExpr(assignmentExpr) {}
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class FunctionDeclarationStmtAST : public StmtAST
{
public:
	const IdentifierExprAST &id;
	VariableList arguments;
	BlockExprAST &block;
	FunctionDeclarationStmtAST(const IdentifierExprAST &id, const VariableList &arguments, BlockExprAST &block) : id(id), arguments(arguments), block(block){};
	virtual llvm::Value *codeGen(CodeGenContext &context);
};

class ReturnStmtAST : public StmtAST
{
public:
	ExprAST &expression;
	ReturnStmtAST(ExprAST &expression) : expression(expression){};
	virtual llvm::Value *codeGen(CodeGenContext &contest);
};
