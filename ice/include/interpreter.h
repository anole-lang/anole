#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"

class Interpreter
{
private:
    Program *program;
public:
    Interpreter();
    void run();
};