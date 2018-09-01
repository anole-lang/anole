#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>
#include "Token.h"

using ::std::string;
using ::std::vector;
using ::std::shared_ptr;

using TOKEN = Ice::Token::TOKEN;

namespace Ice
{

	using StatementList = vector<shared_ptr<class Stmt>>;
	using ExpressionList = vector<shared_ptr<class Expr>>;
	using VariableList = vector<shared_ptr<class VariableDeclarationStmt>>;
	using IdentifierList = vector<shared_ptr<class IdentifierExpr>>;

    class IceObject;

    extern void runUsingStmt(string &, shared_ptr<class Env> &);


	class Node
	{
	public:

		virtual ~Node() {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env>&, shared_ptr<Env> normal_top = nullptr) = 0;
	};


	class Stmt : public Node {};
	class Expr : public Node {};


	class BlockExpr : public Expr
	{
	public:

		StatementList statements;
		BlockExpr() = default;
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class NoneExpr : public Expr
	{
	public:

		NoneExpr() = default;
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class IntegerExpr : public Expr
	{
	public:

		long value;
		IntegerExpr(long value) : value(value) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class DoubleExpr : public Expr
	{
	public:

		double value;
		DoubleExpr(double value) : value(value) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class BooleanExpr : public Expr
	{
	public:

		bool value;
		BooleanExpr(bool value) : value(value) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class StringExpr : public Expr
	{
	public:

		string value;
		StringExpr(string value) : value(value) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class IdentifierExpr : public Expr
	{
	public:

		string name;
		IdentifierExpr(string &name) : name(name) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class MethodCallExpr : public Expr
	{
	public:

		shared_ptr<Expr> expression;
		ExpressionList arguments;
		MethodCallExpr(shared_ptr<Expr> expression, ExpressionList arguments) : expression(expression), arguments(arguments) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class UnaryOperatorExpr : public Expr
	{
	public:

		TOKEN op;
		shared_ptr<Expr> expression;
		UnaryOperatorExpr(TOKEN op, shared_ptr<Expr> expression) : op(op), expression(expression) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class BinaryOperatorExpr : public Expr
	{
	public:

		shared_ptr<Expr> lhs;
		TOKEN op;
		shared_ptr<Expr> rhs;
		BinaryOperatorExpr(shared_ptr<Expr> lhs, TOKEN op, shared_ptr<Expr> rhs) : lhs(lhs), op(op), rhs(rhs) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class LambdaExpr : public Expr
	{
	public:

		VariableList argDecls;
		shared_ptr<BlockExpr> block;
		LambdaExpr(VariableList argDecls, shared_ptr<BlockExpr> block) : argDecls(argDecls), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class NewExpr : public Expr
	{
	public:

		shared_ptr<IdentifierExpr> id;
		ExpressionList arguments;
		NewExpr(shared_ptr<IdentifierExpr> id, ExpressionList arguments) : id(id), arguments(arguments) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class DotExpr : public Expr
	{
	public:

		shared_ptr<Expr> left;
		shared_ptr<Expr> right;
		DotExpr(shared_ptr<Expr> left, shared_ptr<Expr> right) : left(left), right(right) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class EnumExpr : public Expr
	{
	public:

		IdentifierList	enumerators;
		EnumExpr(IdentifierList enumerators) : enumerators(enumerators) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class MatchExpr : public Expr
	{
	public:

		shared_ptr<Expr> expression;
		ExpressionList mat_expressions;
		ExpressionList ret_expressions;
		shared_ptr<Expr> else_expression;
		MatchExpr(shared_ptr<Expr> expression, ExpressionList mat_expressions, ExpressionList ret_expressions, shared_ptr<Expr> else_expression) : expression(expression), mat_expressions(mat_expressions), ret_expressions(ret_expressions), else_expression(else_expression) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ListExpr : public Expr
	{
	public:

		ExpressionList expressions;
		ListExpr(ExpressionList expressions) : expressions(expressions) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class IndexExpr : public Expr
	{
	public:

		shared_ptr<Expr> expression;
		shared_ptr<Expr> index;
		IndexExpr(shared_ptr<Expr> expression, shared_ptr<Expr> index) : expression(expression), index(index) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class DictExpr : public Expr
	{
	public:

		ExpressionList keys;
		ExpressionList values;
		DictExpr() {}
		DictExpr(ExpressionList keys, ExpressionList values) : keys(keys), values(values) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class UsingStmt : public Stmt
	{
	public:

		string name;
		UsingStmt(string name) : name(name) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ExprStmt : public Stmt
	{
	public:

		shared_ptr<Expr> assignment;
		ExprStmt(shared_ptr<Expr> assignment) : assignment(assignment) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class VariableDeclarationStmt : public Stmt
	{
	public:

		shared_ptr<IdentifierExpr> id;
		shared_ptr<Expr> assignment;
		VariableDeclarationStmt(shared_ptr<IdentifierExpr> id, shared_ptr<Expr>assignment) : id(id), assignment(assignment) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class VariableAssignStmt : public Stmt
	{
	public:

		shared_ptr<IdentifierExpr> id;
		shared_ptr<Expr> assignment;
		VariableAssignStmt(shared_ptr<IdentifierExpr> id, shared_ptr<Expr>assignment) : id(id), assignment(assignment) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class FunctionDeclarationStmt : public Stmt
	{
	public:

		shared_ptr<IdentifierExpr> id;
		VariableList argDecls;
		shared_ptr<BlockExpr> block;
		FunctionDeclarationStmt(shared_ptr<IdentifierExpr> id, VariableList argDecls, shared_ptr<BlockExpr> block) : id(id), argDecls(argDecls), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ClassDeclarationStmt : public Stmt
	{
	public:

		shared_ptr<IdentifierExpr> id;
		IdentifierList bases;
		shared_ptr<BlockExpr> block;
		ClassDeclarationStmt(shared_ptr<IdentifierExpr> id, IdentifierList bases, shared_ptr<BlockExpr> block) : id(id), bases(bases), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class BreakStmt : public Stmt
	{
	public:

		BreakStmt() {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ContinueStmt : public Stmt
	{
	public:

		ContinueStmt() {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ReturnStmt : public Stmt
	{
	public:

		shared_ptr<Expr> assignment;
		ReturnStmt(shared_ptr<Expr> assignment) :assignment(assignment) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class IfElseStmt : public Stmt
	{
	public:

		shared_ptr<Expr> cond;
		shared_ptr<BlockExpr> blockTrue;
		shared_ptr<IfElseStmt> elseStmt;
		IfElseStmt(shared_ptr<Expr> cond, shared_ptr<BlockExpr> blockTrue, shared_ptr<IfElseStmt> elseStmt) : cond(cond), blockTrue(blockTrue), elseStmt(elseStmt) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class WhileStmt : public Stmt
	{
	public:

		shared_ptr<Expr> cond;
		shared_ptr<BlockExpr> block;
		WhileStmt(shared_ptr<Expr> cond, shared_ptr<BlockExpr> block) : cond(cond), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class DoWhileStmt : public Stmt
	{
	public:

		shared_ptr<Expr> cond;
		shared_ptr<BlockExpr> block;
		DoWhileStmt(shared_ptr<Expr> cond, shared_ptr<BlockExpr> block) : cond(cond), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ForStmt : public Stmt
	{
	public:

		shared_ptr<Expr> begin;
		shared_ptr<Expr> end;
		shared_ptr<IdentifierExpr> id;
		shared_ptr<BlockExpr> block;
		ForStmt(shared_ptr<Expr> begin, shared_ptr<Expr> end, shared_ptr<IdentifierExpr> id, shared_ptr<BlockExpr> block) : begin(begin), end(end), id(id), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class ForeachStmt : public Stmt
	{
	public:

		shared_ptr<Expr> expression;
		shared_ptr<IdentifierExpr> id;
		shared_ptr<BlockExpr> block;
		ForeachStmt(shared_ptr<Expr> expression, shared_ptr<IdentifierExpr> id, shared_ptr<BlockExpr> block) : expression(expression), id(id), block(block) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class DotStmt : public Stmt
	{
	public:

		ExpressionList expressions;
		shared_ptr<Stmt> to_run;
		DotStmt(ExpressionList expressions, shared_ptr<Stmt> to_run) : expressions(expressions), to_run(to_run) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};


	class IndexStmt : public Stmt
	{
	public:

		shared_ptr<Expr> expression;
		shared_ptr<Expr> index;
		shared_ptr<Expr> assignment;
		IndexStmt(shared_ptr<IndexExpr> index_expr, shared_ptr<Expr>assignment) : expression(index_expr->expression), index(index_expr->index), assignment(assignment) {}
		virtual shared_ptr<IceObject> runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top = nullptr);
	};
}

#endif //__NODE_H__