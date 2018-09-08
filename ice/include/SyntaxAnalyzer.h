#ifndef __SYNTAX_ANALYZER_H__
#define __SYNTAX_ANALYZER_H__


namespace Ice
{
    using BLOCK_EXPR = ::std::shared_ptr<BlockExpr>;
    using STMT = ::std::shared_ptr<Stmt>;
    using EXPR = ::std::shared_ptr<Expr>;


	class SyntaxAnalyzer
	{

	private:

		LexicalAnalyzer lexicalAnalyzer;
		::std::vector<Token>::iterator iToken;
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
		::std::shared_ptr<Node> getNode();
		::std::shared_ptr<Node> getNode(::std::string);
	};
}

#endif //__SYNTAX_ANALYZER_H__