#include "SyntaxAnalyzer.h"

auto SyntaxAnalyzer::cont(Symbol sym) -> Node *(*)
{
    switch(sym)
    {
        case Symbol::stmt:
            break;
        // ...
        default:
            break;
    }
}

Stmt *SyntaxAnalyzer::getNode()
{
    Stmt *node = nullptr;
    return node;
}