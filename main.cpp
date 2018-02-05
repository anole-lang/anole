#include <iostream>
#include "codegen.h"
#include "node.h"

extern NBlock* programBlock;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    CodeGenContext context;
    context.generateCode(*programBlock);
    context.runCode();
    return 0;
}
