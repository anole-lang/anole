#include "SyntaxAnalyzer.h"

Stmt *SyntaxAnalyzer::getNode()
{
    Stmt *node = nullptr;
    int i = 0;
    auto &token = tokens[i++];
    if ( token.token_id == Token::TOKEN::TINTEGER )
    {
        node = new ExprStmt(new IntegerExpr((long)std::stoi(token.value)));
    }
    else if ( token.token_id == Token::TOKEN::TIDENTIFIER && i < (int)tokens.size())
    {
        IdentifierExpr *idexpr = new IdentifierExpr(token.value);
        token = tokens[i++];
        if (token.token_id == Token::TOKEN::TASSIGN)
        {
            token = tokens[i++];
            if(token.token_id == Token::TOKEN::TINTEGER)
            {
                node = new VariableDeclarationStmt(idexpr, new IntegerExpr((long)std::stoi(token.value)));
            }
        }
    }
    else if (token.token_id == Token::TOKEN::TIDENTIFIER)
    {
        node = new ExprStmt(new IdentifierExpr(token.value));
    }
    return node;
}