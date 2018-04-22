#include "SyntaxAnalyzer.h"

namespace Ice
{
	SyntaxAnalyzer::SyntaxAnalyzer()
	{
		genNode[Symbol::stmt] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TAT:
				node = genNode[Symbol::var_decl_or_func_decl]();
				break;
			case Token::TOKEN::TRETURN:
				node = genNode[Symbol::return_stmt]();
				break;
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TINTEGER:
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
					args.push_back(std::make_shared<IdentifierExpr>(iToken->value));
					iToken++;
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

		genNode[Symbol::if_else] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::return_stmt] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			iToken++;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
			case Token::TOKEN::TINTEGER:
			case Token::TOKEN::TLPAREN:
				node = std::make_shared<ReturnStmt>(std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]()));
				break;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::block] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			if (iToken->token_id != Token::TOKEN::TLBRACE) iToken = lexicalAnalyzer.cont();
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
				case Token::TOKEN::TIDENTIFIER:
				case Token::TOKEN::TINTEGER:
				case Token::TOKEN::TLPAREN:
				case Token::TOKEN::TRETURN:
					node->statements.push_back(std::dynamic_pointer_cast<Stmt>(genNode[Symbol::stmt]()));
					break;
				case Token::TOKEN::TEND:
					iToken = lexicalAnalyzer.cont();
					break;
				default:
					std::cout << "missing '{'" << std::endl;
					exit(0);
				}
			}
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
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TINTEGER:
				goto factor;
			default:
				return node;
			};
		factor:
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
			case Token::TOKEN::TLPAREN:
			case Token::TOKEN::TINTEGER:
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
			case Token::TOKEN::TLPAREN:
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
				node = genNode[Symbol::numeric]();
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
				return node;
			default:
				break;
			}
			return node;
		};

		genNode[Symbol::comparison] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::if_else_tail] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};
	}

	std::shared_ptr<Node> SyntaxAnalyzer::getNode()
	{
		auto &tokens = lexicalAnalyzer.getTokens();
		iToken = tokens.begin();
		return (iToken == tokens.end()) ? nullptr : genNode[Symbol::stmt]();
	}
}
