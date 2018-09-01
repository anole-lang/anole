#ifndef __SYNTAX_ANALYZER_H__
#define __SYNTAX_ANALYZER_H__

#include <iostream>
#include <vector>
#include <memory>
#include <stack>
#include <iterator>
#include <functional>
#include <map>
#include "Token.h"
#include "Node.h"
#include "LexicalAnalyzer.h"


using ::std::shared_ptr;
using ::std::make_shared;
using ::std::vector;


using BLOCK_EXPR = shared_ptr<Ice::BlockExpr>;
using STMT = shared_ptr<Ice::Stmt>;
using EXPR = shared_ptr<Ice::Expr>;


namespace Ice
{
	class SyntaxAnalyzer
	{

	private:

		LexicalAnalyzer lexicalAnalyzer;
		vector<Token>::iterator iToken;
		void updateiToken();

		BLOCK_EXPR genStmts();
		BLOCK_EXPR genBlock();
		BLOCK_EXPR genFuncDeclRest();

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

		EXPR genExpr();
		EXPR genLogicOr();
		EXPR genLogicAnd();
		EXPR genLogicNot();
		EXPR genCmp();
		EXPR genFactor();
		EXPR genTerm();
		EXPR genIdentOrOther();
		EXPR genIdent();
		EXPR genNumeric();
		EXPR genBoolean();
		EXPR genString();
		EXPR genEnumOrDict();
		EXPR genLambdaExpr();
		EXPR genNewExpr();
		EXPR genMatchExpr();
		EXPR genListExpr();

	public:

		SyntaxAnalyzer() {}
		shared_ptr<Node> getNode();
		shared_ptr<Node> getNode(::std::string);
	};
}

#endif //__SYNTAX_ANALYZER_H__