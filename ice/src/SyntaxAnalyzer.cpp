#include "SyntaxAnalyzer.h"

Stmt *SyntaxAnalyzer::getNode()
{
    Stmt *node = nullptr;
    auto token = tokens.back();
    if ( token.token_id == Token::TOKEN::TINTEGER )
    {
        node = new ExprStmt(new IntegerExpr((long)std::stoi(token.value)));
    }
    return node;
}