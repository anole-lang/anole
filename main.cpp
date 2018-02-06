#include <iostream>
#include "codegen.h"
#include "node.h"

extern BlockExprAST* programBlock;
extern int yyparse();

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
    yyparse();
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
    CodeGenContext context;
    createCoreFunctions(context);
    context.generateCode(*programBlock);
    context.runCode();
    return 0;
}
