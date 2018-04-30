#include "SyntaxAnalyzer.h"

namespace Ice
{
	SyntaxAnalyzer::SyntaxAnalyzer()
	{
		genNode[Symbol::stmts] = [&]() {
			std::shared_ptr<BlockExpr> node = std::make_shared<BlockExpr>();
			while (iToken->token_id != Token::TOKEN::TEND)
			{
				switch (iToken->token_id)
				{
				case Token::TOKEN::TAT:
				case Token::TOKEN::TUSING:
				case Token::TOKEN::TIDENTIFIER:
				case Token::TOKEN::TINTEGER:
				case Token::TOKEN::TDOUBLE:
				case Token::TOKEN::TTRUE:
				case Token::TOKEN::TFALSE:
				case Token::TOKEN::TSTRING:
				case Token::TOKEN::TSUB:
				case Token::TOKEN::TLPAREN:
				case Token::TOKEN::TIF:
				case Token::TOKEN::TWHILE:
				case Token::TOKEN::TDO:
				case Token::TOKEN::TFOR:
				case Token::TOKEN::TBREAK:
				case Token::TOKEN::TCONTINUE:
				case Token::TOKEN::TRETURN:
					node->statements.push_back(std::dynamic_pointer_cast<Stmt>(genNode[Symbol::stmt]()));
					break;
				default:
					std::cout << "syntax error" << std::endl;
					exit(0);
				}
			}
			return node;
		};

		genNode[Symbol::stmt] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TAT:
				if ((iToken+1)->token_id==Token::TOKEN::TIDENTIFIER)
					node = genNode[Symbol::var_decl_or_func_decl]();
				else node = genNode[Symbol::expr]();
				break;
			case Token::TOKEN::TUSING:
				node = genNode[Symbol::using_stmt]();
				break;
			case Token::TOKEN::TIF:
				node = genNode[Symbol::if_else]();
				break;
			case Token::TOKEN::TWHILE:
				node = genNode[Symbol::while_stmt]();
				break;
			case Token::TOKEN::TDO:
				node = genNode[Symbol::do_while_stmt]();
				break;
			case Token::TOKEN::TFOR:
				node = genNode[Symbol::for_stmt]();
				break;
			case Token::TOKEN::TBREAK:
				iToken++;
				node = std::make_shared<BreakStmt>();
				break;
			case Token::TOKEN::TCONTINUE:
				iToken++;
				node = std::make_shared<ContinueStmt>();
				break;
			case Token::TOKEN::TRETURN:
				node = genNode[Symbol::return_stmt]();
				break;
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TSUB:
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
			case Token::TOKEN::TSTRING:
			case Token::TOKEN::TLPAREN:
				node = std::make_shared<ExprStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::var_decl_or_func_decl] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			iToken++;
			std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
			switch (iToken->token_id)
			{
			case Token::TOKEN::TASSIGN:
				iToken++;
				goto varDecl;
			case Token::TOKEN::TLPAREN:
				iToken++;
				goto funcDecl;
			default:
				std::cout << "missing ':' or '('" << std::endl;
				exit(0);
			}
		varDecl:
			node = std::make_shared<VariableDeclarationStmt>(id, std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
			return node;
		funcDecl:
			std::function<VariableList()> genArguments;
			genArguments = [&]() {
				VariableList args;
				while (iToken->token_id == Token::TOKEN::TIDENTIFIER)
				{
					std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
					std::shared_ptr<Expr> expression = std::make_shared<NoneExpr>();
					if (iToken->token_id == Token::TOKEN::TASSIGN)
					{
						iToken++;
						expression = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
					}
					args.push_back(std::make_shared<VariableDeclarationStmt>(id, expression));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				return args;
			};

			VariableList arguments = genArguments();
			switch (iToken->token_id)
			{
			case Token::TOKEN::TRPAREN:
				iToken++;
				break;
			default:
				std::cout << "missing ')'" << std::endl;
				exit(0);
			}
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::func_decl_rest]());
			node = std::make_shared<FunctionDeclarationStmt>(id, arguments, block);
			return node;
		};

		genNode[Symbol::block] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			updateiToken();
			if (iToken->token_id != Token::TOKEN::TLBRACE)
			{
				std::cout << "missing symbol '{'" << std::endl;
				exit(0);
			}
			iToken++;
			node = genNode[Symbol::block_tail]();
			return node;
		};

		genNode[Symbol::block_tail] = [&]() {
			std::shared_ptr<BlockExpr> node = std::make_shared<BlockExpr>();
			while (iToken->token_id != Token::TOKEN::TRBRACE)
			{
				switch (iToken->token_id)
				{
				case Token::TOKEN::TAT:
				case Token::TOKEN::TUSING:
				case Token::TOKEN::TIDENTIFIER:
				case Token::TOKEN::TINTEGER:
				case Token::TOKEN::TDOUBLE:
				case Token::TOKEN::TTRUE:
				case Token::TOKEN::TFALSE:
				case Token::TOKEN::TSTRING:
				case Token::TOKEN::TSUB:
				case Token::TOKEN::TLPAREN:
				case Token::TOKEN::TIF:
				case Token::TOKEN::TWHILE:
				case Token::TOKEN::TDO:
				case Token::TOKEN::TFOR:
				case Token::TOKEN::TBREAK:
				case Token::TOKEN::TCONTINUE:
				case Token::TOKEN::TRETURN:
					node->statements.push_back(std::dynamic_pointer_cast<Stmt>(genNode[Symbol::stmt]()));
					break;
				case Token::TOKEN::TEND:
					updateiToken();
					break;
				default:
					std::cout << "missing '}'" << std::endl;
					exit(0);
				}
			}
			iToken++;
			return node;
		};

		genNode[Symbol::func_decl_rest] = [&]() {
			std::shared_ptr<BlockExpr> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TASSIGN:
				iToken++;
				node = std::make_shared<BlockExpr>();
				node->statements.push_back(std::make_shared<ReturnStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]())));
				break;
			default:
				node = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
				break;
			}
			return node;
		};

		genNode[Symbol::ident] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
				node = std::make_shared<IdentifierExpr>(iToken->value);
				iToken++;
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::numeric] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TINTEGER:
				node = std::make_shared<IntegerExpr>(std::stoi(iToken->value));
				iToken++;
				break;
			case Token::TOKEN::TDOUBLE:
				node = std::make_shared<DoubleExpr>(std::stod(iToken->value));
				iToken++;
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::boolean] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TTRUE:
				node = std::make_shared<BooleanExpr>(true);
				iToken++;
				break;
			case Token::TOKEN::TFALSE:
				node = std::make_shared<BooleanExpr>(false);
				iToken++;
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::string] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TSTRING:
				node = std::make_shared<StringExpr>(iToken->value);
				iToken++;
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::expr] = [&]() {
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genCmpRest;
			genCmpRest = [&](std::shared_ptr<Expr> lhs) {
				std::shared_ptr<Node> node = lhs;
				Token::TOKEN op;
				switch (iToken->token_id)
				{
				case Token::TOKEN::TCEQ:
				case Token::TOKEN::TCNE:
				case Token::TOKEN::TCLT:
				case Token::TOKEN::TCLE:
				case Token::TOKEN::TCGT:
				case Token::TOKEN::TCGE:
					op = iToken->token_id;
					iToken++;
					break;
				default:
					return node;
				}
				std::shared_ptr<Expr> rhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::cmp]());
				std::shared_ptr<Expr> _lhs = std::make_shared<BinaryOperatorExpr>(lhs, op, rhs);
				node = genCmpRest(_lhs);
				return node;
			};

			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TSUB:
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
			case Token::TOKEN::TSTRING:
			case Token::TOKEN::TAT:
				goto cmp;
			default:
				return node;
			};
		cmp:
			std::shared_ptr<Expr> lhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::cmp]());
			node = genCmpRest(lhs);
			return node;
		};

		genNode[Symbol::cmp] = [&]() {
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genFactorRest;
			genFactorRest = [&](std::shared_ptr<Expr> lhs) {
				std::shared_ptr<Node> node = lhs;
				Token::TOKEN op;
				switch (iToken->token_id)
				{
				case Token::TOKEN::TADD:
				case Token::TOKEN::TSUB:
					op = iToken->token_id;
					iToken++;
					break;
				default:
					return node;
				}
				std::shared_ptr<Expr> rhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::factor]());
				std::shared_ptr<Expr> _lhs = std::make_shared<BinaryOperatorExpr>(lhs, op, rhs);
				node = genFactorRest(_lhs);
				return node;
			};

			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TSUB:
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
			case Token::TOKEN::TSTRING:
			case Token::TOKEN::TAT:
				goto factor;
			default:
				return node;
			};
		factor:
			std::shared_ptr<Expr> lhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::factor]());
			node = genFactorRest(lhs);
			return node;
		};

		genNode[Symbol::factor] = [&]() {
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genTermRest;
			genTermRest = [&](std::shared_ptr<Expr> lhs) {
				std::shared_ptr<Node> node = lhs;
				Token::TOKEN op;
				switch (iToken->token_id)
				{
				case Token::TOKEN::TMUL:
				case Token::TOKEN::TDIV:
				case Token::TOKEN::TMOD:
					op = iToken->token_id;
					iToken++;
					break;
				default:
					return node;
				}
				std::shared_ptr<Expr> rhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::term]());
				std::shared_ptr<Expr> _lhs = std::make_shared<BinaryOperatorExpr>(lhs, op, rhs);
				node = genTermRest(_lhs);
				return node;
			};

			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
			case Token::TOKEN::TSTRING:
			case Token::TOKEN::TSUB:
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TAT:
				goto item;
			default:
				return node;
			}
		item:
			std::shared_ptr<Expr> lhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::term]());
			node = genTermRest(lhs);
			return node;
		};

		genNode[Symbol::term] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
				node = genNode[Symbol::ident]();
				if (iToken->token_id == Token::TOKEN::TLPAREN)
				{
					ExpressionList args;
					iToken++;
					while (iToken->token_id != Token::TOKEN::TRPAREN)
					{
						args.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
						if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
						else break;
					}
					iToken++;
					node = std::make_shared<MethodCallExpr>(std::dynamic_pointer_cast<IdentifierExpr>(node), args);
				}
				break;
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
				node = genNode[Symbol::numeric]();
				break;
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
				node = genNode[Symbol::boolean]();
				break;
			case Token::TOKEN::TSTRING:
				node = genNode[Symbol::string]();
				break;
			case Token::TOKEN::TLPAREN:
				iToken++;
				node = genNode[Symbol::expr]();
				if (iToken->token_id == Token::TOKEN::TRPAREN)
					iToken++;
				else
				{
					std::cout << "missing symbol '(' ." << std::endl;
					exit(0);
				}
				break;
			case Token::TOKEN::TSUB:
				iToken++;
				node = std::make_shared<UnaryOperatorExpr>(Token::TOKEN::TSUB, std::dynamic_pointer_cast<Expr>(genNode[Symbol::term]()));
				break;
			case Token::TOKEN::TAT:
				node = genNode[Symbol::lambda_expr]();
				break;
			default:
				break;
			}
			return node;
		};
		
		genNode[Symbol::if_else] = [&]() {
			iToken++;
			std::shared_ptr<Expr> cond = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			std::shared_ptr<BlockExpr> blockTrue = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			std::shared_ptr<BlockExpr> blockFalse = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::if_else_tail]());
			return std::dynamic_pointer_cast<Node>(std::make_shared<IfElseStmt>(cond, blockTrue, blockFalse));
		};

		genNode[Symbol::if_else_tail] = [&]() {
			auto node = std::make_shared<BlockExpr>();
			updateiToken();
			switch (iToken->token_id)
			{
			case Token::TOKEN::TELSE:
				iToken++;
				return std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			default:
				break;
			}
			return node;
		};
		
		genNode[Symbol::while_stmt] = [&]() {
			iToken++;
			std::shared_ptr<Expr> cond = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			return std::dynamic_pointer_cast<Node>(std::make_shared<WhileStmt>(cond, block));
		};

		genNode[Symbol::do_while_stmt] = [&]() {
			iToken++;
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			if (iToken->token_id != Token::TOKEN::TWHILE)
			{
				std::cout << "missing keyword 'while' after 'do'" << std::endl;
				exit(0);
			}
			iToken++;
			std::shared_ptr<Expr> cond = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			return std::dynamic_pointer_cast<Node>(std::make_shared<DoWhileStmt>(cond, block));
		};

		genNode[Symbol::for_stmt] = [&]() {
			iToken++;
			std::shared_ptr<Expr> begin = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			if (iToken->token_id != Token::TOKEN::TTO)
			{
				std::cout << "missing keyword 'to' in for" << std::endl;
				exit(0);
			}
			iToken++;
			std::shared_ptr<Expr> end = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			return std::dynamic_pointer_cast<Node>(std::make_shared<ForStmt>(begin, end, block));
		};
		
		genNode[Symbol::return_stmt] = [&]() {
			iToken++;
			return std::dynamic_pointer_cast<Node>(std::make_shared<ReturnStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]())));
		};

		genNode[Symbol::using_stmt] = [&]() {
			iToken++;
			if (iToken->token_id != Token::TOKEN::TIDENTIFIER)
			{
				std::cout << "missing an identifier after 'using'" << std::endl;
				exit(0);
			}
			return std::dynamic_pointer_cast<Node>(std::make_shared<UsingStmt>((iToken++)->value));
		};

		genNode[Symbol::lambda_expr] = [&]() {
			std::function<VariableList()> genArguments;
			genArguments = [&]() {
				VariableList args;
				while (iToken->token_id == Token::TOKEN::TIDENTIFIER)
				{
					std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
					std::shared_ptr<Expr> expression = std::make_shared<NoneExpr>();
					if (iToken->token_id == Token::TOKEN::TASSIGN)
					{
						iToken++;
						expression = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
					}
					args.push_back(std::make_shared<VariableDeclarationStmt>(id, expression));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				return args;
			};

			std::shared_ptr<Node> node = nullptr;
			iToken++;
			if (iToken->token_id != Token::TOKEN::TLPAREN)
			{
				std::cout << "missing symbol '('" << std::endl;
				exit(0);
			}
			iToken++;
			VariableList args = genArguments();
			if (iToken->token_id != Token::TOKEN::TRPAREN)
			{
				std::cout << "missing symbol ')'" << std::endl;
				exit(0);
			}
			iToken++;
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			if (iToken->token_id == Token::TOKEN::TLPAREN)
			{
				ExpressionList exprs;
				iToken++;
				while (iToken->token_id != Token::TOKEN::TRPAREN)
				{
					exprs.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				iToken++;
				node = std::make_shared<LambdaCallExpr>(args, block, exprs);
			}
			else node = std::make_shared<LambdaExpr>(args, block);
			return node;
		};
	}

	void SyntaxAnalyzer::updateiToken()
	{
		if (iToken->token_id == Token::TOKEN::TEND) iToken = lexicalAnalyzer.cont();
	}

	std::shared_ptr<Node> SyntaxAnalyzer::getNode()
	{
		std::string line;
		std::getline(std::cin, line);
		line += "$";
		auto &tokens = lexicalAnalyzer.getTokens(line);
		iToken = tokens.begin();
		return (iToken == tokens.end()) ? nullptr : genNode[Symbol::stmt]();
	}

	std::shared_ptr<Node> SyntaxAnalyzer::getNode(std::string code)
	{
		auto &tokens = lexicalAnalyzer.getTokens(code);
		iToken = tokens.begin();
		return (iToken == tokens.end()) ? nullptr : genNode[Symbol::stmts]();
	}
}
