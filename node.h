#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class StmtAST;
class ExprAST;
class VariableDeclarationStmtAST;

typedef std::vector<StmtAST*> StatementList;
typedef std::vector<ExprAST*> ExpressionList;
typedef std::vector<VariableDeclarationStmtAST*> VariableList;

class Node{
public:
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class StmtAST: public Node{};

class ExprAST: public Node{};

class IntegerExprAST: public ExprAST{
public:
	long long value;
	IntegerExprAST(long long value):value(value){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class DoubleExprAST: public ExprAST{
public:
	double value;
	DoubleExprAST(double value):value(value){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class IdentifierExprAST: public ExprAST{
public:
	std::string name;
	IdentifierExprAST(std::string& name):name(name){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class AssignmentExprAST: public ExprAST{
public:
	IdentifierExprAST& lhs;
	ExprAST& rhs;
	AssignmentExprAST(IdentifierExprAST& lhs, ExprAST& rhs):
		lhs(lhs), rhs(rhs){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BinaryOperatorExprAST: public ExprAST{
public:
	int op;
	ExprAST& lhs;
	ExprAST& rhs;
	BinaryOperatorExprAST(ExprAST& lhs, int op, ExprAST& rhs):
		lhs(lhs), op(op), rhs(rhs){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BlockExprAST: public ExprAST{
public:
	StatementList statements;
	BlockExprAST(){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ExprStmtAST: public StmtAST {
public:
    ExprAST& expression;
    ExprStmtAST(ExprAST& expression) : 
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class VariableDeclarationStmtAST: public StmtAST {
public:
	const IdentifierExprAST& type;
	IdentifierExprAST& id;
	ExprAST *assignmentExpr;
	VariableDeclarationStmtAST(const IdentifierExprAST& type, IdentifierExprAST& id):
		type(type), id(id){}
	VariableDeclarationStmtAST(const IdentifierExprAST& type, IdentifierExprAST& id, ExprAST *assignmentExpr):
		type(type), id(id), assignmentExpr(assignmentExpr){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class PrintStmtAST: public StmtAST{
public:
	ExprAST& arg;
	PrintStmtAST(ExprAST& arg): arg(arg){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};