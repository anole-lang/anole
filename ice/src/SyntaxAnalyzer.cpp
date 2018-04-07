#include "SyntaxAnalyzer.h"

Node *SyntaxAnalyzer::getNode()
{
    Node *node;
    node = new Stmt();
    return node;
}