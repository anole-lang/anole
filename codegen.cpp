#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

void CodeGenContext::generateCode(NBlock& root)
{
    std::cout << "Generating code..." << std::endl;

    vector<Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(MyContext), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", mainFunction, 0);
    
    pushBlock(bblock);
    root.codeGen(*this);
    ReturnInst::Create(MyContext, bblock);
    popBlock();

    std::cout << "Code is generated!" << std::endl;
    legacy::PassManager pm;
    pm.add(createPrintModulePass(outs()));
    pm.run(*module);
}

GenericValue CodeGenContext::runCode()
{
    std::cout << "==========================" << std::endl;
    std::cout << "Running code..." << std::endl;
	ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run." << std::endl;
	return v;
}

static Type *typeOf(const NIdentifier& type)
{
    if(type.name.compare("int") == 0)
    {
        return Type::getInt64Ty(MyContext);
    }
    else if(type.name.compare("double") == 0)
    {
        return Type::getDoubleTy(MyContext);
    }
    return Type::getVoidTy(MyContext);
}

Value* NInteger::codeGen(CodeGenContext& context)
{
    std::cout << "Creating" << value << std::endl;
    return ConstantInt::get(Type::getInt64Ty(MyContext), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
    std::cout << "Creating " << value << std::endl;
    return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
    std::cout << "Creating identifier reference: " << name << std::endl;
    if (context.locals().find(name) == context.locals().end())
    {
        std::cerr << "undeclared variable " << name << std::endl;
        return NULL;
    }
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* NAssignment::codeGen(CodeGenContext& context)
{
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    if(context.locals().find(lhs.name) == context.locals().end())
    {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        return NULL;
    }
    return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for(it = statements.begin(); it != statements.end(); it++)
    {
        std::cout << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).codeGen(context);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(expression).name() << std::endl;
    return expression.codeGen(context);
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;
    AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
    context.locals()[id.name] = alloc;
    if (assignmentExpr != NULL)
    {
        NAssignment assn(id, *assignmentExpr);
        assn.codeGen(context);
    }
    return alloc;
}