#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <cstdio>
#include "Token.h"
#include "LexicalAnalyzer.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"

namespace Ice
{
class BlockExpr;

class Interpreter
{
  private:
    std::shared_ptr<Env> top;
    std::shared_ptr<BlockExpr> block;
    
    LexicalAnalyzer lexicalAnalyzer;
    SyntaxAnalyzer syntaxAnalyzer;

  public:
    Interpreter();
    void run();
};
}

#endif //__INTERPRETER_H__
