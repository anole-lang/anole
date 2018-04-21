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
			std::shared_ptr<Expr> assignment = std::dynamic_pointer_cast<Expr>(genNode[Symbol::expr]());
			node = std::make_shared<VariableDeclarationStmt>(id, assignment);
			return node;
		funcDecl:
			VariableList arguments; // TODO: complete lambda genArguments;
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
			return node;
		};

		genNode[Symbol::block] = [&]() { // TODO: complete
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::block_tail] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::var_decl_tail] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::func_decl_tail] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::func_decl_rest] = [&]() { // TODO: complete
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::func_decl_args] = [&]() {
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::func_decl_args_tail] = [&]() {
			std::shared_ptr<Node> node = nullptr;
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

		genNode[Symbol::term] = [&]() { // TODO: add method_call_tail
			std::shared_ptr<Node> node = nullptr;
			switch (iToken->token_id)
			{
			case Token::TOKEN::TIDENTIFIER:
				node = genNode[Symbol::ident]();
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

		genNode[Symbol::method_call_tail] = [&]() { // TODO: complete
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::call_args] = [&]() { // TODO: complete
			std::shared_ptr<Node> node = nullptr;
			return node;
		};

		genNode[Symbol::call_args_tail] = [&]() { // TODO: complete
			std::shared_ptr<Node> node = nullptr;
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
