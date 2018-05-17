#include "SyntaxAnalyzer.h"

namespace Ice
{
	SyntaxAnalyzer::SyntaxAnalyzer()
	{
		genNode[Symbol::stmts] = [&]() {
			std::shared_ptr<BlockExpr> node = std::make_shared<BlockExpr>();
			while (iToken->token_id != Token::TOKEN::TEND)
			{
				node->statements.push_back(std::dynamic_pointer_cast<Stmt>(genNode[Symbol::stmt]()));
			}
			return node;
		};

		genNode[Symbol::stmt] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TAT:
				if ((iToken + 1)->token_id == Token::TOKEN::TLPAREN)
					node = std::make_shared<ExprStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				else
					node = genNode[Symbol::decl_or_assign]();
				break;
			case Token::TOKEN::TATAT:
				node = genNode[Symbol::class_decl]();
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
			case Token::TOKEN::TNONE:
			case Token::TOKEN::TTRUE:
			case Token::TOKEN::TFALSE:
			case Token::TOKEN::TSTRING:
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TNEW:
			case Token::TOKEN::TMATCH:
			case Token::TOKEN::TLBRACKET:
			case Token::TOKEN::TNOT:
				node = std::make_shared<ExprStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::decl_or_assign] = [&]() { 
			std::function<VariableList()> genArguments;
			std::function<std::shared_ptr<Node>(ExpressionList &)> genDeclWithDot;
			std::function<std::shared_ptr<IndexExpr>(std::shared_ptr<Expr>)> genIndexExpr;

			genArguments = [&]() {
				VariableList args;
				iToken++;
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
				iToken++;
				return args;
			};

			genDeclWithDot = [&](ExpressionList &expressions) {
				std::string type;
				iToken++;
				std::shared_ptr<Node> node = genNode[Symbol::ident]();
				if (iToken->token_id == Token::TOKEN::TASSIGN)
				{
					iToken++;
					node = std::make_shared<VariableDeclarationStmt>(std::dynamic_pointer_cast<IdentifierExpr>(node), std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				}
				else if (iToken->token_id == Token::TOKEN::TLPAREN)
				{
					VariableList args = genArguments();
					node = std::make_shared<FunctionDeclarationStmt>(std::dynamic_pointer_cast<IdentifierExpr>(node), args, std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::func_decl_rest]()));
				}
				else if (iToken->token_id == Token::TOKEN::TLBRACKET)
				{
					while (iToken->token_id == Token::TOKEN::TLBRACKET)
						node = genIndexExpr(std::dynamic_pointer_cast<Expr>(node));
					if (iToken->token_id == Token::TOKEN::TASSIGN)
					{
						iToken++;
						node = std::make_shared<IndexStmt>(std::dynamic_pointer_cast<IndexExpr>(node), std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					}
					else if (iToken->token_id == Token::TOKEN::TDOT)
					{
						expressions.push_back(std::dynamic_pointer_cast<Expr>(node));
						node = genDeclWithDot(expressions);
					}
					else
					{
						std::cout << "missing ':' or '('" << std::endl;
						exit(0);
					}
				}
				else if (iToken->token_id == Token::TOKEN::TDOT)
				{
					ExpressionList expressions;
					expressions.push_back(std::dynamic_pointer_cast<Expr>(node));
					node = genDeclWithDot(expressions);
				}
				else
				{
					std::cout << "missing ':' or '('" << std::endl;
					exit(0);
				}
				return std::dynamic_pointer_cast<Node>(std::make_shared<DotStmt>(expressions, std::dynamic_pointer_cast<Stmt>(node), type));
			};

			genIndexExpr = [&](std::shared_ptr<Expr> expression) {
				iToken++;
				std::shared_ptr<Expr> node = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
				if (iToken->token_id != Token::TOKEN::TRBRACKET)
				{
					std::cout << "missing symbol ']'" << std::endl;
					exit(0);
				}
				iToken++;
				return std::make_shared<IndexExpr>(expression, node);
			};

			std::shared_ptr<Node> node = nullptr;
			iToken++;
			if (iToken->token_id == Token::TOKEN::TDOT)
			{
				return genNode[Symbol::var_assign]();
			}

			node = genNode[Symbol::ident]();
			if (iToken->token_id == Token::TOKEN::TASSIGN)
			{
				iToken++;
				node = std::make_shared<VariableDeclarationStmt>(std::dynamic_pointer_cast<IdentifierExpr>(node), std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
			}
			else if (iToken->token_id == Token::TOKEN::TLPAREN)
			{
				VariableList args = genArguments();
				node = std::make_shared<FunctionDeclarationStmt>(std::dynamic_pointer_cast<IdentifierExpr>(node), args, std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::func_decl_rest]()));
			}
			else if (iToken->token_id == Token::TOKEN::TLBRACKET)
			{
				while (iToken->token_id == Token::TOKEN::TLBRACKET)
					node = genIndexExpr(std::dynamic_pointer_cast<Expr>(node));
				if (iToken->token_id == Token::TOKEN::TASSIGN)
				{
					iToken++;
					node = std::make_shared<IndexStmt>(std::dynamic_pointer_cast<IndexExpr>(node), std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				}
				else if (iToken->token_id == Token::TOKEN::TDOT)
				{
					ExpressionList expressions;
					expressions.push_back(std::dynamic_pointer_cast<Expr>(node));
					node = genDeclWithDot(expressions);
				}
				else
				{
					std::cout << "missing ':' or '('" << std::endl;
					exit(0);
				}
			}
			else if (iToken->token_id == Token::TOKEN::TDOT)
			{
				ExpressionList expressions;
				expressions.push_back(std::dynamic_pointer_cast<Expr>(node));
				node = genDeclWithDot(expressions);
			}
			else
			{
				std::cout << "missing ':' or '(' after @ident" << std::endl;
				exit(0);
			}
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
				if (iToken->token_id == Token::TOKEN::TEND) updateiToken();
				node->statements.push_back(std::dynamic_pointer_cast<Stmt>(genNode[Symbol::stmt]()));
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
			std::shared_ptr<Node> node = std::make_shared<IdentifierExpr>(iToken->value);
			iToken++;
			return node;
		};

		genNode[Symbol::numeric] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			if (iToken->token_id == Token::TOKEN::TINTEGER)
				node = std::make_shared<IntegerExpr>(std::stoi(iToken->value));
			else
				node = std::make_shared<DoubleExpr>(std::stod(iToken->value));
			iToken++;
			return node;
		};

		genNode[Symbol::boolean] = [&]() {
			std::shared_ptr<Node> node = std::make_shared<BooleanExpr>(((iToken->token_id == Token::TOKEN::TTRUE) ? true : false));
			iToken++;
			return node;
		};

		genNode[Symbol::string] = [&]() {
			std::shared_ptr<Node> node = std::make_shared<StringExpr>(iToken->value);
			iToken++;
			return node;
		};

		genNode[Symbol::expr] = [&]() {
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genLogicOrRest;
			genLogicOrRest = [&](std::shared_ptr<Expr> lhs) {
				std::shared_ptr<Node> node = lhs;
				Token::TOKEN op;
				switch (iToken->token_id)
				{
				case Token::TOKEN::TOR:
					op = iToken->token_id;
					iToken++;
					break;
				default:
					return node;
				}
				std::shared_ptr<Expr> rhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::logic_or]());
				std::shared_ptr<Expr> _lhs = std::make_shared<BinaryOperatorExpr>(lhs, op, rhs);
				node = genLogicOrRest(_lhs);
				return node;
			};

			std::shared_ptr<Node> node = nullptr;
			if (iToken->token_id == Token::TOKEN::TLBRACE)
			{
				node = genNode[Symbol::enum_expr]();
				return node;
			}

			node = genNode[Symbol::logic_or]();
			node = genLogicOrRest(std::dynamic_pointer_cast<Expr>(node));
			return node;
		};

		genNode[Symbol::logic_or] = [&](){
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genLogicAndRest;
			genLogicAndRest = [&](std::shared_ptr<Expr> lhs) {
				std::shared_ptr<Node> node = lhs;
				Token::TOKEN op;
				switch (iToken->token_id)
				{
				case Token::TOKEN::TAND:
					op = iToken->token_id;
					iToken++;
					break;
				default:
					return node;
				}
				std::shared_ptr<Expr> rhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::logic_and]());
				std::shared_ptr<Expr> _lhs = std::make_shared<BinaryOperatorExpr>(lhs, op, rhs);
				node = genLogicAndRest(_lhs);
				return node;
			};

			std::shared_ptr<Node> node = genNode[Symbol::logic_and]();
			node = genLogicAndRest(std::dynamic_pointer_cast<Expr>(node));
			return node;
		};

		genNode[Symbol::logic_and] = [&](){
			return genNode[Symbol::logic_not]();
		};

		genNode[Symbol::logic_not] = [&]() {
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
			if (iToken->token_id == Token::TOKEN::TNOT)
			{
				iToken++;
				node = genNode[Symbol::cmp]();
				node = genCmpRest(std::dynamic_pointer_cast<Expr>(node));
				node = std::make_shared<UnaryOperatorExpr>(Token::TOKEN::TNOT, std::dynamic_pointer_cast<Expr>(node));
				return node;
			}
			node = genNode[Symbol::cmp]();
			node = genCmpRest(std::dynamic_pointer_cast<Expr>(node));
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

			std::shared_ptr<Expr> lhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::factor]());
			return genFactorRest(lhs);
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

			std::shared_ptr<Expr> lhs = std::dynamic_pointer_cast<Expr>(genNode[Symbol::term]());
			return genTermRest(lhs);
		};

		genNode[Symbol::term] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
				node = genNode[Symbol::ident_or_other]();
				break;
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TDOUBLE:
				node = genNode[Symbol::numeric]();
				break;
			case Token::TOKEN::TNONE:
				iToken++;
				node = std::make_shared<NoneExpr>();
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
			case Token::TOKEN::TNEW:
				node = genNode[Symbol::new_expr]();
				break;
			case Token::TOKEN::TMATCH:
				node = genNode[Symbol::match_expr]();
				break;
			case Token::TOKEN::TLBRACKET:
				node = genNode[Symbol::list_expr]();
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::ident_or_other] = [&]() {
			std::function<ExpressionList()> genArgs;
			std::function<std::shared_ptr<Node>(std::shared_ptr<Expr>)> genDotExpr;
			std::function<std::shared_ptr<IndexExpr>(std::shared_ptr<Expr>)> genIndexExpr;

			genArgs = [&]() {
				ExpressionList args;
				iToken++;
				while (iToken->token_id != Token::TOKEN::TRPAREN)
				{
					args.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
				}
				iToken++;
				return args;
			};

			genDotExpr = [&](std::shared_ptr<Expr> left) {
				iToken++;
				std::shared_ptr<Node> node = genNode[Symbol::ident]();
				if (iToken->token_id == Token::TOKEN::TLPAREN)
					node = std::make_shared<MethodCallExpr>(std::dynamic_pointer_cast<Expr>(node), genArgs());
				else if (iToken->token_id == Token::TOKEN::TDOT)
					node = genDotExpr(std::dynamic_pointer_cast<Expr>(node));
				else if (iToken->token_id == Token::TOKEN::TLBRACKET)
					node = genIndexExpr(std::dynamic_pointer_cast<Expr>(node));
				return std::make_shared<DotExpr>(left, std::dynamic_pointer_cast<Expr>(node));
			};

			genIndexExpr = [&](std::shared_ptr<Expr> expression) {
				iToken++;
				std::shared_ptr<Expr> node = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
				if (iToken->token_id != Token::TOKEN::TRBRACKET)
				{
					std::cout << "missing symbol ']'" << std::endl;
					exit(0);
				}
				iToken++;
				return std::make_shared<IndexExpr>(expression, node);
			};


			std::shared_ptr<Node> node = genNode[Symbol::ident]();

			while (iToken->token_id == Token::TOKEN::TLPAREN || iToken->token_id == Token::TOKEN::TDOT || iToken->token_id == Token::TOKEN::TLBRACKET)
			{
				if (iToken->token_id == Token::TOKEN::TLPAREN)
					node = std::make_shared<MethodCallExpr>(std::dynamic_pointer_cast<Expr>(node), genArgs());
				else if (iToken->token_id == Token::TOKEN::TDOT)
					node = genDotExpr(std::dynamic_pointer_cast<Expr>(node));
				else if (iToken->token_id == Token::TOKEN::TLBRACKET)
					node = genIndexExpr(std::dynamic_pointer_cast<Expr>(node));
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

		genNode[Symbol::class_decl] = [&]() {
			std::function<IdentifierList()> genBases;
			genBases = [&]() {
				iToken++;
				IdentifierList bases;
				while (iToken->token_id == Token::TOKEN::TIDENTIFIER)
				{
					bases.push_back(std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				if (iToken->token_id != Token::TOKEN::TRPAREN)
				{
					std::cout << "missing symbol ')'" << std::endl;
					exit(0);
				}
				iToken++;
				return bases;
			};

			std::shared_ptr<Node> node = nullptr;
			iToken++;
			std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
			if (iToken->token_id != Token::TOKEN::TLPAREN)
			{
				std::cout << "missing symbol '('" << std::endl;
				exit(0);
			}
			IdentifierList bases = genBases();
			std::shared_ptr<BlockExpr> block = std::dynamic_pointer_cast<BlockExpr>(genNode[Symbol::block]());
			node = std::make_shared<ClassDeclarationStmt>(id, bases, block);
			return node;
		};

		genNode[Symbol::new_expr] = [&]() {
			std::function<ExpressionList()> genArgs;
			genArgs = [&]() {
				ExpressionList args;
				iToken++;
				while (iToken->token_id != Token::TOKEN::TRPAREN)
				{
					args.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				if (iToken->token_id != Token::TOKEN::TRPAREN)
				{
					std::cout << "missing symbol ')'" << std::endl;
					exit(0);
				}
				iToken++;
				return args;
			};

			iToken++;
			std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
			if (iToken->token_id != Token::TOKEN::TLPAREN)
			{
				std::cout << "missing symbol '('" << std::endl;
				exit(0);
			}
			ExpressionList args = genArgs();
			return std::make_shared<NewExpr>(id, args);
		};

		genNode[Symbol::var_assign] = [&]() {
			iToken++;
			std::shared_ptr<IdentifierExpr> id = std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]());
			if (iToken->token_id != Token::TOKEN::TASSIGN)
			{
				std::cout << "missing symbol ':'" << std::endl;
				exit(0);
			}
			iToken++;
			std::shared_ptr<Expr> assignment = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			return std::make_shared<VariableAssignStmt>(id, assignment);
		};

		genNode[Symbol::enum_expr] = [&]() {
			std::function<IdentifierList()> genEnumerators;
			genEnumerators = [&]() {
				iToken++;
				IdentifierList enumerators;
				while (iToken->token_id == Token::TOKEN::TIDENTIFIER)
				{
					enumerators.push_back(std::dynamic_pointer_cast<IdentifierExpr>(genNode[Symbol::ident]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
					else break;
				}
				if (iToken->token_id != Token::TOKEN::TRBRACE)
				{
					std::cout << "missing symbol '}'" << std::endl;
					exit(0);
				}
				iToken++;
				return enumerators;
			};

			return std::make_shared<EnumExpr>(genEnumerators());
		};

		genNode[Symbol::match_expr] = [&]() {
			iToken++;
			std::shared_ptr<Expr> expression = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			ExpressionList mat_expressions;
			ExpressionList ret_expressions;
			std::shared_ptr<Expr> else_expression = std::make_shared<NoneExpr>();
			if (iToken->token_id != Token::TOKEN::TLBRACE)
			{
				std::cout << "missing symbol '{'" << std::endl;
				exit(0);
			}
			iToken++;
			while (iToken->token_id != Token::TOKEN::TRBRACE)
			{
				int counter = 1;
				mat_expressions.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));

				while (iToken->token_id == Token::TOKEN::TCOMMA)
				{
					iToken++;
					mat_expressions.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					counter++;
				}

				if (iToken->token_id != Token::TOKEN::TRET)
				{
					std::cout << "missing symbol '=>'" << std::endl;
					exit(0);
				}
				iToken++;

				std::shared_ptr<Expr> ret_expression = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
				while (counter--)	
					ret_expressions.push_back(ret_expression);

				if (iToken->token_id == Token::TOKEN::TCOMMA)
				{
					iToken++;
				}
			}
			iToken++;
			if (iToken->token_id == Token::TOKEN::TELSE)
			{
				iToken++;
				else_expression = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			}
			return std::make_shared<MatchExpr>(expression, mat_expressions, ret_expressions, else_expression);
		};

		genNode[Symbol::list_expr] = [&]() {
			std::function<ExpressionList()> genArgs;
			genArgs = [&]() {
				ExpressionList args;
				while (iToken->token_id != Token::TOKEN::TRBRACKET)
				{
					args.push_back(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
					if (iToken->token_id == Token::TOKEN::TCOMMA) iToken++;
				}
				iToken++;
				return args;
			};

			iToken++;
			return std::make_shared<ListExpr>(genArgs());
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
