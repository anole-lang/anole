#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node{
public:
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class NStatement: public Node{};

class NExpression: public Node{};

class NInteger: public NExpression{
public:
	long long value;
	NInteger(long long value):value(value){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble: public NExpression{
public:
	double value;
	NDouble(double value):value(value){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier: public NExpression{
public:
	std::string name;
	NIdentifier(std::string& name):name(name){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment: public NExpression{
public:
	NIdentifier& lhs;
	NExpression& rhs;
	NAssignment(NIdentifier& lhs, NExpression& rhs):
		lhs(lhs), rhs(rhs){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator: public NExpression{
public:
	int op;
	NExpression& lhs;
	NExpression& rhs;
	NBinaryOperator(NExpression& lhs, int op, NExpression& rhs):
		lhs(lhs), op(op), rhs(rhs){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock: public NExpression{
public:
	StatementList statements;
	NBlock(){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) : 
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration: public NStatement{
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id):
		type(type), id(id){}
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr):
		type(type), id(id), assignmentExpr(assignmentExpr){}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};