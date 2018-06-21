#include "SyntaxAnalyzer.h"

using std::cin;
using std::cout;
using std::endl;
using std::atoi;
using std::atof;
using std::getline;
using std::string;
using std::function;
using std::dynamic_pointer_cast;

using TOKEN = Ice::Token::TOKEN;

#define THROW(MESSAGE)						{ cout << MESSAGE << endl; exit(0); }
#define CHECK_AND_THROW(TOKEN_ID, MESSAGE)	if (iToken->token_id != TOKEN_ID) \
											{ cout << MESSAGE << endl; exit(0); }

namespace Ice
{
	// update iToken when cannot find the next token
	void SyntaxAnalyzer::updateiToken()
	{ 
		if (iToken->token_id == TOKEN::TEND) iToken = lexicalAnalyzer.cont();
	}

	// usually use when interacting
	BLOCK_EXPR SyntaxAnalyzer::genStmts()
	{ 
		auto stmts = make_shared<BlockExpr>();
		while (iToken->token_id != TOKEN::TEND)
		{
			stmts->statements.push_back(genStmt());
		}
		return stmts;
	}

	// gen normal block as {...}
	BLOCK_EXPR SyntaxAnalyzer::genBlock()
	{ 
		auto block = make_shared<BlockExpr>();
		updateiToken();
		CHECK_AND_THROW(TOKEN::TLBRACE, "missing symbol '{'");
		++iToken; // skip '{'

		// '}' means the end of a block
		while (iToken->token_id != TOKEN::TRBRACE)
		{
			updateiToken();
			auto stmt = genStmt();
			if (stmt != nullptr)
			{
				block->statements.push_back(stmt);
			}
		}
		++iToken; // skip '}'
		return block;
	}

	// generate else block after if {}
	BLOCK_EXPR SyntaxAnalyzer::genIfElseTail()
	{ 
		updateiToken();
		if (iToken->token_id == TOKEN::TELSE)
		{
			++iToken;
			return genBlock();
		}
		else
		{
			return make_shared<BlockExpr>();
		}
	}

	// generate function block (: expr | {}) after @foo()
	BLOCK_EXPR SyntaxAnalyzer::genFuncDeclRest()
	{ 
		if (iToken->token_id == TOKEN::TASSIGN)
		{
			++iToken;
			auto block = make_shared<BlockExpr>();
			block->statements.push_back(make_shared<ReturnStmt>(genExpr()));
			return block;
		}
		else if (iToken->token_id == TOKEN::TLBRACE)
		{
			return genBlock();
		}
		else THROW("missing symbol ':' or '{' after @func()");
	}

	// generate normal statement
	STMT SyntaxAnalyzer::genStmt()
	{ 
		switch (iToken->token_id)
		{
		case TOKEN::TAT:
			if ((iToken + 1)->token_id == TOKEN::TLPAREN)
				return make_shared<ExprStmt>(genExpr());
			else
				return genDeclOrAssign();
			break;
		case TOKEN::TATAT:
			return genClassDecl();
		case TOKEN::TUSING:
			return genUsingStmt();
		case TOKEN::TIF:
			return genIfElse();
		case TOKEN::TWHILE:
			return genWhileStmt();
		case TOKEN::TDO:
			return genDoWhileStmt();
		case TOKEN::TFOR:
			return genForStmt();
		case TOKEN::TBREAK:
			++iToken;
			return make_shared<BreakStmt>();
		case TOKEN::TCONTINUE:
			++iToken;
			return make_shared<ContinueStmt>();
		case TOKEN::TRETURN:
			return genReturnStmt();
		case TOKEN::TIDENTIFIER:
		case TOKEN::TSUB:
		case TOKEN::TINTEGER:
		case TOKEN::TDOUBLE:
		case TOKEN::TNONE:
		case TOKEN::TTRUE:
		case TOKEN::TFALSE:
		case TOKEN::TSTRING:
		case TOKEN::TLPAREN:
		case TOKEN::TNEW:
		case TOKEN::TMATCH:
		case TOKEN::TLBRACKET:
		case TOKEN::TNOT:
			return make_shared<ExprStmt>(genExpr());
		default:
			return nullptr;
		}
	}

	// generate declaration or assignment (@.var:)
	STMT SyntaxAnalyzer::genDeclOrAssign()
	{ 
		// generate arguments defined as (a, b:1)
		function<VariableList()> genArguments;
		function<shared_ptr<Node>(ExpressionList &)> genDeclWithDot;
		function<shared_ptr<IndexExpr>(shared_ptr<Expr>)> genIndexExpr;
		
		genArguments = [&]() 
		{ 
			VariableList args;
			++iToken; // skip '('
			while (iToken->token_id == TOKEN::TIDENTIFIER)
			{
				auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
				shared_ptr<Expr> expression = make_shared<NoneExpr>();
				if (iToken->token_id == TOKEN::TASSIGN)
				{
					++iToken;
					expression = genExpr();
				}
				args.push_back(make_shared<VariableDeclarationStmt>(id, expression));
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			++iToken; // skip ')'
			return args;
		};

		genDeclWithDot = [&](ExpressionList &expressions) 
		{
			++iToken;
			shared_ptr<Node> node = genIdent();
			if (iToken->token_id == TOKEN::TASSIGN)
			{
				++iToken;
				node = make_shared<VariableDeclarationStmt>(dynamic_pointer_cast<IdentifierExpr>(node), genExpr());
			}
			else if (iToken->token_id == TOKEN::TLPAREN)
			{
				VariableList args = genArguments();
				node = make_shared<FunctionDeclarationStmt>(dynamic_pointer_cast<IdentifierExpr>(node), args, genFuncDeclRest());
			}
			else if (iToken->token_id == TOKEN::TLBRACKET)
			{
				while (iToken->token_id == TOKEN::TLBRACKET)
				{
					node = genIndexExpr(dynamic_pointer_cast<Expr>(node));
				}
				if (iToken->token_id == TOKEN::TASSIGN)
				{
					++iToken;
					node = make_shared<IndexStmt>(dynamic_pointer_cast<IndexExpr>(node), genExpr());
				}
				else if (iToken->token_id == TOKEN::TDOT)
				{
					expressions.push_back(dynamic_pointer_cast<Expr>(node));
					node = genDeclWithDot(expressions);
				}
				else THROW("missing ':' or '('");
			}
			else if (iToken->token_id == TOKEN::TDOT)
			{
				ExpressionList expressions;
				expressions.push_back(dynamic_pointer_cast<Expr>(node));
				node = genDeclWithDot(expressions);
			}
			else THROW("missing ':' or '('");
			return make_shared<DotStmt>(expressions, dynamic_pointer_cast<Stmt>(node));
		};

		genIndexExpr = [&](shared_ptr<Expr> expression) 
		{
			++iToken;
			auto node = genExpr();
			CHECK_AND_THROW(TOKEN::TRBRACKET, "missing symbol ']'");
			++iToken;
			return make_shared<IndexExpr>(expression, node);
		};

		shared_ptr<Node> node = nullptr;
		++iToken;
		if (iToken->token_id == TOKEN::TDOT)
		{
			return genVarAssign();
		}

		node = genIdent();
		if (iToken->token_id == TOKEN::TASSIGN)
		{
			++iToken;
			node = make_shared<VariableDeclarationStmt>(dynamic_pointer_cast<IdentifierExpr>(node), genExpr());
		}
		else if (iToken->token_id == TOKEN::TLPAREN)
		{
			VariableList args = genArguments();
			node = make_shared<FunctionDeclarationStmt>(dynamic_pointer_cast<IdentifierExpr>(node), args, genFuncDeclRest());
		}
		else if (iToken->token_id == TOKEN::TLBRACKET)
		{
			while (iToken->token_id == TOKEN::TLBRACKET)
			{
				node = genIndexExpr(dynamic_pointer_cast<Expr>(node));
			}
			if (iToken->token_id == TOKEN::TASSIGN)
			{
				++iToken;
				node = make_shared<IndexStmt>(dynamic_pointer_cast<IndexExpr>(node), genExpr());
			}
			else if (iToken->token_id == TOKEN::TDOT)
			{
				ExpressionList expressions;
				expressions.push_back(dynamic_pointer_cast<Expr>(node));
				node = genDeclWithDot(expressions);
			}
			else THROW("missing ':' or '('");
		}
		else if (iToken->token_id == TOKEN::TDOT)
		{
			ExpressionList expressions;
			expressions.push_back(dynamic_pointer_cast<Expr>(node));
			node = genDeclWithDot(expressions);
		}
		else THROW("missing ':' or '(' after @ident");
		return dynamic_pointer_cast<Stmt>(node);
	}

	// generate assignment as @.ident: expr
	STMT SyntaxAnalyzer::genVarAssign()
	{
		++iToken;
		auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
		CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
		++iToken;
		auto assignment = genExpr();
		return make_shared<VariableAssignStmt>(id, assignment);
	}

	STMT SyntaxAnalyzer::genClassDecl()
	{
		auto genBases = [&]() 
		{
			++iToken;
			IdentifierList bases;
			while (iToken->token_id == TOKEN::TIDENTIFIER)
			{
				bases.push_back(dynamic_pointer_cast<IdentifierExpr>(genIdent()));
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			CHECK_AND_THROW(TOKEN::TRPAREN, "missing symbol ')'");
			++iToken;
			return bases;
		};

		shared_ptr<Node> node = nullptr;
		++iToken;
		auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
		CHECK_AND_THROW(TOKEN::TLPAREN, "missing symbol '('");
		auto bases = genBases();
		auto block = genBlock();
		return make_shared<ClassDeclarationStmt>(id, bases, block);
	}

	STMT SyntaxAnalyzer::genUsingStmt()
	{
		++iToken;
		CHECK_AND_THROW(TOKEN::TIDENTIFIER, "missing an identifier after 'using'");
		return make_shared<UsingStmt>((iToken++)->value);
	}

	STMT SyntaxAnalyzer::genIfElse()
	{
		++iToken;
		auto cond = genExpr();
		auto blockTrue = genBlock();
		auto blockFalse = genIfElseTail();
		return make_shared<IfElseStmt>(cond, blockTrue, blockFalse);
	}

	STMT SyntaxAnalyzer::genWhileStmt()
	{
		++iToken;
		auto cond = genExpr();
		auto block = genBlock();
		return make_shared<WhileStmt>(cond, block);
	}

	STMT SyntaxAnalyzer::genDoWhileStmt()
	{
		++iToken;
		auto block = genBlock();
		CHECK_AND_THROW(TOKEN::TWHILE, "missing keyword 'while' after 'do'");
		++iToken;
		auto cond = genExpr();
		return make_shared<DoWhileStmt>(cond, block);
	}

	STMT SyntaxAnalyzer::genForStmt()
	{
		++iToken;
		auto begin = genExpr();
		CHECK_AND_THROW(TOKEN::TTO, "missing keyword 'to' in for");
		++iToken;
		auto end = genExpr();
		auto block = genBlock();
		return make_shared<ForStmt>(begin, end, block);
	}

	STMT SyntaxAnalyzer::genReturnStmt()
	{
		++iToken;
		return make_shared<ReturnStmt>(genExpr());
	}

	EXPR SyntaxAnalyzer::genExpr()
	{
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genLogicOrRest = [&](shared_ptr<Expr> lhs) 
		{
			auto node = lhs;
			TOKEN op;
			switch (iToken->token_id)
			{
			case TOKEN::TOR:
				op = iToken->token_id;
				++iToken;
				break;
			default:
				return node;
			}
			auto rhs = genLogicOr();
			shared_ptr<Expr> _lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
			node = genLogicOrRest(_lhs);
			return node;
		};

		shared_ptr<Expr> node = nullptr;
		if (iToken->token_id == TOKEN::TLBRACE)
		{
			return genEnumOrDict();;
		}

		node = genLogicOr();
		return genLogicOrRest(node);
	}

	EXPR SyntaxAnalyzer::genLogicOr()
	{
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genLogicAndRest = [&](shared_ptr<Expr> lhs) 
		{
			shared_ptr<Expr> node = lhs;
			TOKEN op;
			switch (iToken->token_id)
			{
			case TOKEN::TAND:
				op = iToken->token_id;
				++iToken;
				break;
			default:
				return node;
			}
			shared_ptr<Expr> rhs = genLogicAnd();
			shared_ptr<Expr> _lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
			node = genLogicAndRest(_lhs);
			return node;
		};

		auto node = genLogicAnd();
		return genLogicAndRest(node);
	}

	EXPR SyntaxAnalyzer::genLogicAnd()
	{
		return genLogicNot();
	}

	EXPR SyntaxAnalyzer::genLogicNot()
	{
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genCmpRest = [&](shared_ptr<Expr> lhs)
		{
			shared_ptr<Expr> node = lhs;
			TOKEN op;
			switch (iToken->token_id)
			{
			case TOKEN::TCEQ:
			case TOKEN::TCNE:
			case TOKEN::TCLT:
			case TOKEN::TCLE:
			case TOKEN::TCGT:
			case TOKEN::TCGE:
				op = iToken->token_id;
				++iToken;
				break;
			default:
				return node;
			}
			shared_ptr<Expr> rhs = genCmp();
			shared_ptr<Expr> _lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
			node = genCmpRest(_lhs);
			return node;
		};

		shared_ptr<Expr> node = nullptr;
		if (iToken->token_id == TOKEN::TNOT)
		{
			++iToken;
			node = genCmp();
			node = genCmpRest(node);
			return make_shared<UnaryOperatorExpr>(TOKEN::TNOT, node);
		}
		node = genCmp();
		return genCmpRest(node);
	}

	EXPR SyntaxAnalyzer::genCmp()
	{
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genFactorRest = [&](shared_ptr<Expr> lhs)
		{
			shared_ptr<Expr> node = lhs;
			TOKEN op;
			switch (iToken->token_id)
			{
			case TOKEN::TADD:
			case TOKEN::TSUB:
				op = iToken->token_id;
				++iToken;
				break;
			default:
				return node;
			}
			shared_ptr<Expr> rhs = genFactor();
			shared_ptr<Expr> _lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
			node = genFactorRest(_lhs);
			return node;
		};

		auto lhs = genFactor();
		return genFactorRest(lhs);
	}

	EXPR SyntaxAnalyzer::genFactor()
	{
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genTermRest = [&](shared_ptr<Expr> lhs)
		{
			shared_ptr<Expr> node = lhs;
			TOKEN op;
			switch (iToken->token_id)
			{
			case TOKEN::TMUL:
			case TOKEN::TDIV:
			case TOKEN::TMOD:
				op = iToken->token_id;
				++iToken;
				break;
			default:
				return node;
			}
			shared_ptr<Expr> rhs = genTerm();
			shared_ptr<Expr> _lhs = make_shared<BinaryOperatorExpr>(lhs, op, rhs);
			node = genTermRest(_lhs);
			return node;
		};

		auto lhs = genTerm();
		return genTermRest(lhs);
	}

	EXPR SyntaxAnalyzer::genTerm()
	{
		shared_ptr<Expr> node = nullptr;
		switch (iToken->token_id)
		{
		case TOKEN::TIDENTIFIER:
			return genIdentOrOther();
		case TOKEN::TINTEGER:
		case TOKEN::TDOUBLE:
			return genNumeric();
		case TOKEN::TNONE:
			++iToken;
			return make_shared<NoneExpr>();
		case TOKEN::TTRUE:
		case TOKEN::TFALSE:
			return genBoolean();
		case TOKEN::TSTRING:
			return genString();
		case TOKEN::TLPAREN:
			++iToken;
			node = genExpr();
			CHECK_AND_THROW(TOKEN::TRPAREN, "missing symbol '(' .");
			++iToken;
			return node;
		case TOKEN::TSUB:
			++iToken;
			return make_shared<UnaryOperatorExpr>(TOKEN::TSUB, genTerm());
		case TOKEN::TAT:
			return genLambdaExpr();
		case TOKEN::TNEW:
			return genNewExpr();
		case TOKEN::TMATCH:
			return genMatchExpr();
		case TOKEN::TLBRACKET:
			return genListExpr();
		default:
			break;
		}
		return node;
	}

	EXPR SyntaxAnalyzer::genIdentOrOther()
	{
		function<ExpressionList()> genArgs;
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genDotExpr;
		function<shared_ptr<IndexExpr>(shared_ptr<Expr>)> genIndexExpr;

		genArgs = [&]() 
		{
			ExpressionList args;
			++iToken;
			while (iToken->token_id != TOKEN::TRPAREN)
			{
				args.push_back(genExpr());
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
			}
			++iToken;
			return args;
		};

		genDotExpr = [&](shared_ptr<Expr> left) 
		{
			++iToken;
			shared_ptr<Expr> node = genIdent();
			if (iToken->token_id == TOKEN::TLPAREN)
				node = make_shared<MethodCallExpr>(node, genArgs());
			else if (iToken->token_id == TOKEN::TDOT)
				node = genDotExpr(node);
			else if (iToken->token_id == TOKEN::TLBRACKET)
				node = genIndexExpr(node);
			return make_shared<DotExpr>(left, node);
		};

		genIndexExpr = [&](shared_ptr<Expr> expression) 
		{
			++iToken;
			shared_ptr<Expr> node = genExpr();
			CHECK_AND_THROW(TOKEN::TRBRACKET, "missing symbol ']'");
			++iToken;
			return make_shared<IndexExpr>(expression, node);
		};

		shared_ptr<Expr> node = genIdent();

		while (iToken->token_id == TOKEN::TLPAREN || iToken->token_id == TOKEN::TDOT || iToken->token_id == TOKEN::TLBRACKET)
		{
			if (iToken->token_id == TOKEN::TLPAREN)
				node = make_shared<MethodCallExpr>(node, genArgs());
			else if (iToken->token_id == TOKEN::TDOT)
				node = genDotExpr(node);
			else if (iToken->token_id == TOKEN::TLBRACKET)
				node = genIndexExpr(node);
		}
		return node;
	}

	EXPR SyntaxAnalyzer::genIdent()
	{
		auto node = make_shared<IdentifierExpr>(iToken->value);
		++iToken;
		return node;
	}

	EXPR SyntaxAnalyzer::genNumeric()
	{
		shared_ptr<Expr> node = nullptr;
		if (iToken->token_id == TOKEN::TINTEGER)
			node = make_shared<IntegerExpr>(stoi(iToken->value));
		else
			node = make_shared<DoubleExpr>(stod(iToken->value));
		++iToken;
		return node;
	}

	EXPR SyntaxAnalyzer::genBoolean()
	{
		auto node = make_shared<BooleanExpr>(((iToken->token_id == TOKEN::TTRUE) ? true : false));
		++iToken;
		return node;
	}

	EXPR SyntaxAnalyzer::genString()
	{
		auto node = make_shared<StringExpr>(iToken->value);
		++iToken;
		return node;
	}

	// generate enum as {NAME1, NAME2, ..., NAMEN} or dict as {KEY1: VAL1, KEY2: VAL2, ..., KEYN: VALN}
	EXPR SyntaxAnalyzer::genEnumOrDict()
	{ 
		function<shared_ptr<Expr>(shared_ptr<IdentifierExpr>)> genEnumExpr;
		function<shared_ptr<Expr>(shared_ptr<Expr>)> genDictExpr;

		genEnumExpr = [&](shared_ptr<IdentifierExpr> first) 
		{
			++iToken;
			IdentifierList enumerators;
			enumerators.push_back(first);
			while (iToken->token_id == TOKEN::TIDENTIFIER)
			{
				enumerators.push_back(dynamic_pointer_cast<IdentifierExpr>(genIdent()));
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			CHECK_AND_THROW(TOKEN::TRBRACE, "missing symbol '}'");
			++iToken;
			return make_shared<EnumExpr>(enumerators);
		};

		genDictExpr = [&](shared_ptr<Expr> first) 
		{
			CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
			ExpressionList keys, values;
			keys.push_back(first);

			++iToken;
			values.push_back(genExpr());
			if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
			while (iToken->token_id != TOKEN::TRBRACE)
			{
				keys.push_back(genExpr());
				CHECK_AND_THROW(TOKEN::TASSIGN, "missing symbol ':'");
				++iToken;
				values.push_back(genExpr());
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			++iToken;
			return make_shared<DictExpr>(keys, values);
		};

		++iToken;
		if (iToken->token_id == TOKEN::TRBRACE)
		{
			++iToken;
			return make_shared<DictExpr>();
		}
		auto node = genExpr();
		if (iToken->token_id == TOKEN::TCOMMA)
		{
			return genEnumExpr(dynamic_pointer_cast<IdentifierExpr>(node));
		}
		else
		{
			return genDictExpr(node);
		}
	}

	// generate lambda expr as @(){} and also @(){}()..
	EXPR SyntaxAnalyzer::genLambdaExpr()
	{ 
		auto genArguments = [&]() 
		{
			VariableList args;
			while (iToken->token_id == TOKEN::TIDENTIFIER)
			{
				auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
				shared_ptr<Expr> expression = make_shared<NoneExpr>();
				if (iToken->token_id == TOKEN::TASSIGN)
				{
					++iToken;
					expression = genExpr();
				}
				args.push_back(make_shared<VariableDeclarationStmt>(id, expression));
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			return args;
		};

		auto genArgsAfterDecl = [&]()
		{
			ExpressionList args;
			++iToken;
			while (iToken->token_id != TOKEN::TRPAREN)
			{
				args.push_back(genExpr());
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
			}
			++iToken;
			return args;
		};

		shared_ptr<Expr> node = nullptr;
		++iToken;
		CHECK_AND_THROW(TOKEN::TLPAREN, "missing symbol '('");
		++iToken;
		VariableList args = genArguments();
		CHECK_AND_THROW(TOKEN::TRPAREN, "missing symbol ')'");
		++iToken;
		auto block = genBlock();
		node = make_shared<LambdaExpr>(args, block);

		while (iToken->token_id == TOKEN::TLPAREN)
		{
			node = make_shared<MethodCallExpr>(node, genArgsAfterDecl());
		}

		return node;
	}

	// generate new expr as @instance: new Class()
	EXPR SyntaxAnalyzer::genNewExpr()
	{ 
		auto genArgs = [&]() 
		{
			ExpressionList args;
			++iToken;
			while (iToken->token_id != TOKEN::TRPAREN)
			{
				args.push_back(genExpr());
				if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
				else break;
			}
			CHECK_AND_THROW(TOKEN::TRPAREN, "missing symbol ')'");
			++iToken;
			return args;
		};

		++iToken;
		auto id = dynamic_pointer_cast<IdentifierExpr>(genIdent());
		CHECK_AND_THROW(TOKEN::TLPAREN, "missing symbol '('");
		ExpressionList args = genArgs();
		return make_shared<NewExpr>(id, args);
	}

	/* EXPR SyntaxAnalyzer::genMatchExpr()
	generate match expr as follow:
		match value {
			1, 2, 3, 4 => "smaller than five",
			5 => "five"
		} else "bigger than five"
	*/
	EXPR SyntaxAnalyzer::genMatchExpr()
	{ 
		++iToken;
		auto expression = genExpr();
		ExpressionList mat_expressions;
		ExpressionList ret_expressions;
		shared_ptr<Expr> else_expression = make_shared<NoneExpr>();
		CHECK_AND_THROW(TOKEN::TLBRACE, "missing symbol '{'");
		++iToken; // skip '{'
		while (iToken->token_id != TOKEN::TRBRACE)
		{
			int counter = 1;
			mat_expressions.push_back(genExpr());

			while (iToken->token_id == TOKEN::TCOMMA)
			{
				++iToken;
				mat_expressions.push_back(genExpr());
				counter++;
			}

			CHECK_AND_THROW(TOKEN::TRET, "missing symbol '=>'");
			++iToken;

			auto ret_expression = genExpr();
			while (counter--)
			{
				ret_expressions.push_back(ret_expression);
			}

			if (iToken->token_id == TOKEN::TCOMMA)
			{
				++iToken;
			}
		}
		++iToken; // skip '}'

		// check 'else' after 'match {}'
		if (iToken->token_id == TOKEN::TELSE)
		{ 
			++iToken;
			else_expression = genExpr();
		}
		return make_shared<MatchExpr>(expression, mat_expressions, ret_expressions, else_expression);
	}

	// generate list as [expr1, expr2, ..., exprN]
	EXPR SyntaxAnalyzer::genListExpr()
	{ 
		++iToken; // skip '['
		ExpressionList expressions;
		while (iToken->token_id != TOKEN::TRBRACKET)
		{
			expressions.push_back(genExpr());
			if (iToken->token_id == TOKEN::TCOMMA) ++iToken;
		}
		++iToken; // skip(']')
		return make_shared<ListExpr>(expressions);
	}

	// use when interacting & return stmt node
	shared_ptr<Node> SyntaxAnalyzer::getNode()
	{ 
		string line;
		getline(cin, line);
		auto &tokens = lexicalAnalyzer.getTokens(line);
		iToken = tokens.begin();
		return (iToken == tokens.end()) ? nullptr : genStmt();
	}

	// use when in interpreting file & return stmts node
	shared_ptr<Node> SyntaxAnalyzer::getNode(string code)
	{ 
		auto &tokens = lexicalAnalyzer.getTokens(code);
		iToken = tokens.begin();
		return (iToken == tokens.end()) ? nullptr : genStmts();
	}
}
