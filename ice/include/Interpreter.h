#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "Token.h"
#include "LexicalAnalyzer.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"

class BlockExpr;

class Interpreter
{
private:
    Env *top;
    BlockExpr *block;

public:
    Interpreter();
    void run();
};

#endif //__INTERPRETER_H__

