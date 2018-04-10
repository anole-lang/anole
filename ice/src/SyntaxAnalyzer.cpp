#include "SyntaxAnalyzer.h"

Stmt *SyntaxAnalyzer::getNode()
{
    Stmt *node = nullptr;
    auto token = tokens.front();
    if ( token.token_id == Token::TOKEN::TINTEGER )
    {
        node = new ExprStmt(new IntegerExpr(std::stoi(token.value)));
    }
    return node;
}