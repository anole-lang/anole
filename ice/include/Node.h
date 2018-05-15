#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>
#include "Token.h"

namespace Ice
{
	class Env;
	class IceObject;

	extern void runUsingStmt(std::string &, std::shared_ptr<Env> &);

	class Stmt;
	class Expr;
	class VariableDeclarationStmt;
	class IdentifierExpr;

	typedef std::vector<std::shared_ptr<Stmt>> StatementList;
	typedef std::vector<std::shared_ptr<Expr>> ExpressionList;
	typedef std::vector<std::shared_ptr<VariableDeclarationStmt>> VariableList;
	typedef std::vector<std::shared_ptr<IdentifierExpr>> IdentifierList;

	class Node
	{
	public:
		virtual ~Node() {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env>&, std::shared_ptr<Env> normal_top = nullptr) = 0;
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
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class NoneExpr : public Expr
	{
	public:
		NoneExpr() {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class IntegerExpr : public Expr
	{
	public:
		long value;
		IntegerExpr(long value) : value(value) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class DoubleExpr : public Expr
	{
	public:
		double value;
		DoubleExpr(double value) : value(value) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class BooleanExpr : public Expr
	{
	public:
		bool value;
		BooleanExpr(bool value) : value(value) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class StringExpr : public Expr
	{
	public:
		std::string value;
		StringExpr(std::string value) : value(value) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class IdentifierExpr : public Expr
	{
	public:
		std::string name;
		IdentifierExpr(std::string &name) : name(name) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class MethodCallExpr : public Expr
	{
	public:
		std::shared_ptr<Expr> expression;
		ExpressionList arguments;
		MethodCallExpr(std::shared_ptr<Expr> expression, ExpressionList arguments) : expression(expression), arguments(arguments) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class UnaryOperatorExpr : public Expr
	{
	public:
		Token::TOKEN op;
		std::shared_ptr<Expr> expression;
		UnaryOperatorExpr(Token::TOKEN op, std::shared_ptr<Expr> expression) : op(op), expression(expression) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class BinaryOperatorExpr : public Expr
	{
	public:
		std::shared_ptr<Expr> lhs;
		Token::TOKEN op;
		std::shared_ptr<Expr> rhs;
		BinaryOperatorExpr(std::shared_ptr<Expr> lhs, Token::TOKEN op, std::shared_ptr<Expr> rhs) : lhs(lhs), op(op), rhs(rhs) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class LambdaExpr : public Expr
	{
	public:
		VariableList argDecls;
		std::shared_ptr<BlockExpr> block;
		LambdaExpr(VariableList argDecls, std::shared_ptr<BlockExpr> block) : argDecls(argDecls), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class LambdaCallExpr : public Expr
	{
	public:
		VariableList argDecls;
		std::shared_ptr<BlockExpr> block;
		ExpressionList expressions;
		LambdaCallExpr(VariableList argDecls, std::shared_ptr<BlockExpr> block, ExpressionList expressions) : argDecls(argDecls), block(block), expressions(expressions) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class NewExpr : public Expr
	{
	public:
		std::shared_ptr<IdentifierExpr> id;
		ExpressionList arguments;
		NewExpr(std::shared_ptr<IdentifierExpr> id, ExpressionList arguments) : id(id), arguments(arguments) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class DotExpr : public Expr
	{
	public:
		std::shared_ptr<Expr> left;
		std::shared_ptr<Expr> right;
		DotExpr(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right) : left(left), right(right) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class EnumExpr : public Expr
	{
	public:
		IdentifierList	enumerators;
		EnumExpr(IdentifierList enumerators) : enumerators(enumerators) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class MatchExpr : public Expr
	{
	public:
		std::shared_ptr<Expr> expression;
		ExpressionList mat_expressions;
		ExpressionList ret_expressions;
		std::shared_ptr<Expr> else_expression;
		MatchExpr(std::shared_ptr<Expr> expression, ExpressionList mat_expressions, ExpressionList ret_expressions, std::shared_ptr<Expr> else_expression) : expression(expression), mat_expressions(mat_expressions), ret_expressions(ret_expressions), else_expression(else_expression) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ListExpr : public Expr
	{
	public:
		ExpressionList expressions;
		ListExpr(ExpressionList expressions) : expressions(expressions) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class IndexExpr : public Expr
	{
	public:
		std::shared_ptr<Expr> expression;
		std::shared_ptr<Expr> index;
		IndexExpr(std::shared_ptr<Expr> expression, std::shared_ptr<Expr> index) : expression(expression), index(index) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class DictExpr : public Expr
	{
	public:
		ExpressionList keys;
		ExpressionList values;
		DictExpr(ExpressionList keys, ExpressionList values) : keys(keys), values(values) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class UsingStmt : public Stmt
	{
	public:
		std::string name;
		UsingStmt(std::string name) : name(name) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ExprStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> assignment;
		ExprStmt(std::shared_ptr<Expr> assignment) : assignment(assignment) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class VariableDeclarationStmt : public Stmt
	{
	public:
		std::shared_ptr<IdentifierExpr> id;
		std::shared_ptr<Expr> assignment;
		VariableDeclarationStmt(std::shared_ptr<IdentifierExpr> id, std::shared_ptr<Expr>assignment) : id(id), assignment(assignment) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class VariableAssignStmt : public Stmt
	{
	public:
		std::shared_ptr<IdentifierExpr> id;
		std::shared_ptr<Expr> assignment;
		VariableAssignStmt(std::shared_ptr<IdentifierExpr> id, std::shared_ptr<Expr>assignment) : id(id), assignment(assignment) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class FunctionDeclarationStmt : public Stmt
	{
	public:
		std::shared_ptr<IdentifierExpr> id;
		VariableList argDecls;
		std::shared_ptr<BlockExpr> block;
		FunctionDeclarationStmt(std::shared_ptr<IdentifierExpr> id, VariableList argDecls, std::shared_ptr<BlockExpr> block) : id(id), argDecls(argDecls), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ClassDeclarationStmt : public Stmt
	{
	public:
		std::shared_ptr<IdentifierExpr> id;
		IdentifierList bases;
		std::shared_ptr<BlockExpr> block;
		ClassDeclarationStmt(std::shared_ptr<IdentifierExpr> id, IdentifierList bases, std::shared_ptr<BlockExpr> block) : id(id), bases(bases), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class BreakStmt : public Stmt
	{
	public:
		BreakStmt() {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ContinueStmt : public Stmt
	{
	public:
		ContinueStmt() {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ReturnStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> assignment;
		ReturnStmt(std::shared_ptr<Expr> assignment) :assignment(assignment) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class IfElseStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> cond;
		std::shared_ptr<BlockExpr> blockTrue;
		std::shared_ptr<BlockExpr> blockFalse;
		IfElseStmt(std::shared_ptr<Expr> cond, std::shared_ptr<BlockExpr> blockTrue, std::shared_ptr<BlockExpr> blockFalse) : cond(cond), blockTrue(blockTrue), blockFalse(blockFalse) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class WhileStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> cond;
		std::shared_ptr<BlockExpr> block;
		WhileStmt(std::shared_ptr<Expr> cond, std::shared_ptr<BlockExpr> block) : cond(cond), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class DoWhileStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> cond;
		std::shared_ptr<BlockExpr> block;
		DoWhileStmt(std::shared_ptr<Expr> cond, std::shared_ptr<BlockExpr> block) : cond(cond), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class ForStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> begin;
		std::shared_ptr<Expr> end;
		std::shared_ptr<BlockExpr> block;
		ForStmt(std::shared_ptr<Expr> begin, std::shared_ptr<Expr> end, std::shared_ptr<BlockExpr> block) : begin(begin), end(end), block(block) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class DotStmt : public Stmt
	{
	public:
		ExpressionList expressions;
		std::shared_ptr<Stmt> to_run;
		std::string type;
		DotStmt(ExpressionList expressions, std::shared_ptr<Stmt> to_run, std::string type) : expressions(expressions), to_run(to_run), type(type) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};

	class IndexStmt : public Stmt
	{
	public:
		std::shared_ptr<Expr> expression;
		std::shared_ptr<Expr> index;
		std::shared_ptr<Expr> assignment;
		IndexStmt(std::shared_ptr<IndexExpr> index_expr, std::shared_ptr<Expr>assignment) : expression(index_expr->expression), index(index_expr->index), assignment(assignment) {}
		virtual std::shared_ptr<IceObject> runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top = nullptr);
	};
}

#endif //__NODE_H__