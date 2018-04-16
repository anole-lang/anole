#include "SyntaxAnalyzer.h"

SyntaxAnalyzer::SyntaxAnalyzer(std::vector<Token> &tokens) : tokens(tokens)
{
    iToken = tokens.begin();

    genNode[Symbol::stmt] = [&](){
        Node *node = nullptr;
        switch (iToken->token_id)
        {
            case Token::TOKEN::TAT:
                node = genNode[Symbol::var_decl_or_func_decl]();
                break;
            case Token::TOKEN::TIDENTIFIER:
                node = new ExprStmt(dynamic_cast<Expr *>(genNode[Symbol::expr]()));
                break;
            default:
                break;
        }
        return node;
    };

    genNode[Symbol::var_decl_or_func_decl] = [&](){
        switch (iToken->token_id)
        {
            case Token::TOKEN::TAT:
                goto varDecl;
            default:
                break;
        }
        varDecl:
        iToken++;
        IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(genNode[Symbol::ident]());
        Expr *assignment = dynamic_cast<Expr *>(genNode[Symbol::var_decl_or_func_decl_tail]());
        return new VariableDeclarationStmt(id, assignment);
    };

    genNode[Symbol::if_else] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::expr] = [&](){
        Expr *node = nullptr;
        switch (iToken->token_id)
        {
            case Token::TOKEN::TINTEGER:
                node = new IntegerExpr(std::stoi(iToken->value));
                iToken++;
                break;
            case Token::TOKEN::TIDENTIFIER:
                node = new IdentifierExpr(iToken->value);
                iToken++;
                break;
            default:
                break;
        };
        return node;
    };

    genNode[Symbol::return_stmt] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::var_decl_or_func_decl_tail] = [&](){
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

    genNode[Symbol::block] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::block_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };
    
    genNode[Symbol::var_decl_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_rest] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_args] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::func_decl_args_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::ident] = [&](){
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

    genNode[Symbol::numeric] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::factor_rest] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::factor] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::term] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::term_rest] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::method_call_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::call_args] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::call_args_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::comparison] = [&](){
        Node *_ = nullptr;
        return _;
    };

    genNode[Symbol::if_else_tail] = [&](){
        Node *_ = nullptr;
        return _;
    };
}

Node *SyntaxAnalyzer::getNode()
{
    Node *node = genNode[Symbol::stmt]();
    return node;
}