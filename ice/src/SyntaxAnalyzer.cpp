#include "SyntaxAnalyzer.h"

SyntaxAnalyzer::SyntaxAnalyzer(std::vector<Token> &tokens) : tokens(tokens)
{
    iToken = tokens.begin();

    genNode[Symbol::stmt] = [&]() {
        Node *node = nullptr;
        switch (iToken->token_id)
        {
        case Token::TOKEN::TAT:
            node = genNode[Symbol::var_decl_or_func_decl]();
            break;
        case Token::TOKEN::TIDENTIFIER:
        case Token::TOKEN::TINTEGER:
        case Token::TOKEN::TLPAREN:
            node = new ExprStmt(dynamic_cast<Expr *>(genNode[Symbol::expr]()));
            break;
        default:
            break;
        }
        return node;
    };

    genNode[Symbol::var_decl_or_func_decl] = [&]() {
        VariableDeclarationStmt *node = nullptr;
        switch (iToken->token_id)
        {
        case Token::TOKEN::TAT:
            goto varDecl;
        default:
            return node;
        }
    varDecl:
        iToken++;
        IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(genNode[Symbol::ident]());
        Expr *assignment = dynamic_cast<Expr *>(genNode[Symbol::var_decl_or_func_decl_tail]());
        node = new VariableDeclarationStmt(id, assignment);
        return node;
    };

    genNode[Symbol::if_else] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::return_stmt] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::var_decl_or_func_decl_tail] = [&]() {
        switch (iToken->token_id)
        {
        case Token::TOKEN::TASSIGN:
            iToken++;
            return genNode[Symbol::expr]();
        default:
            break;
        }
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::block] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::block_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::var_decl_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_rest] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_args] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_args_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::ident] = [&]() {
        Node *node = nullptr;
        switch (iToken->token_id)
        {
        case Token::TOKEN::TIDENTIFIER:
            node = new IdentifierExpr(iToken->value);
            iToken++;
            break;
        default:
            break;
        }
        return node;
    };

    genNode[Symbol::numeric] = [&]() {
        Node *node = nullptr;
        switch (iToken->token_id)
        {
        case Token::TOKEN::TINTEGER:
            node = new IntegerExpr(std::stoi(iToken->value));
            iToken++;
            break;
        default:
            break;
        }
        return node;
    };

    genNode[Symbol::expr] = [&]() {
        std::function<Node *(Expr *)> genCmpRest;
        genCmpRest = [&](Expr *lhs) {
            Node *node = lhs;
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
            Expr *rhs = dynamic_cast<Expr *>(genNode[Symbol::factor]());
            Expr *_lhs = new BinaryOperatorExpr(lhs, op, rhs);
            node = genCmpRest(_lhs);
            return node;
        };
        Node *node = nullptr;
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
        Expr *lhs = dynamic_cast<Expr *>(genNode[Symbol::cmp]());
        node = genCmpRest(lhs);
        return node;
    };

    genNode[Symbol::cmp] = [&]() {
        std::function<Node *(Expr *)> genFactorRest;
        genFactorRest = [&](Expr *lhs) {
            Node *node = lhs;
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
            Expr *rhs = dynamic_cast<Expr *>(genNode[Symbol::factor]());
            Expr *_lhs = new BinaryOperatorExpr(lhs, op, rhs);
            node = genFactorRest(_lhs);
            return node;
        };
        Node *node = nullptr;
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
        Expr *lhs = dynamic_cast<Expr *>(genNode[Symbol::factor]());
        node = genFactorRest(lhs);
        return node;
    };

    genNode[Symbol::factor] = [&]() {
        std::function<Node *(Expr *)> genTermRest;
        genTermRest = [&](Expr *lhs) {
            Node *node = lhs;
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
            Expr *rhs = dynamic_cast<Expr *>(genNode[Symbol::term]());
            Expr *_lhs = new BinaryOperatorExpr(lhs, op, rhs);
            node = genTermRest(_lhs);
            return node;
        };
        Node *node = nullptr;
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
        Expr *lhs = dynamic_cast<Expr *>(genNode[Symbol::term]());
        node = genTermRest(lhs);
        return node;
    };

    genNode[Symbol::term] = [&]() {
        Node *node = nullptr;
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
                std::cout << "missing '(' ." << std::endl;
                exit(0);
            }
            return node;
        default:
            break;
        }
        return node;
    };

    genNode[Symbol::method_call_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::call_args] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::call_args_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::comparison] = [&]() {
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::if_else_tail] = [&]() {
        Node *_ = nullptr;
        return _;
    };
}

Node *SyntaxAnalyzer::getNode()
{
    Node *node = iToken == tokens.end() ? nullptr : genNode[Symbol::stmt]();
    return node;
}