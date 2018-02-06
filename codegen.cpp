#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

void CodeGenContext::generateCode(BlockExprAST &root)
{
    vector<Type *> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(TheContext), makeArrayRef(argTypes), false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
    BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", mainFunction, 0);

    pushBlock(bblock);
    root.codeGen(*this);
    ReturnInst::Create(TheContext, bblock);
    popBlock();

    legacy::PassManager pm;
    //pm.add(createPrintModulePass(outs()));
    pm.run(*module);
}

GenericValue CodeGenContext::runCode()
{
    ExecutionEngine *ee = EngineBuilder(unique_ptr<Module>(module)).create();
    ee->finalizeObject();
    vector<GenericValue> noargs;
    GenericValue v = ee->runFunction(mainFunction, noargs);
    return v;
}

static Type *typeOf(const IdentifierExprAST &type)
{
    if (type.name.compare("int") == 0)
    {
        return Type::getInt64Ty(TheContext);
    }
    else if (type.name.compare("double") == 0)
    {
        return Type::getDoubleTy(TheContext);
    }
    return Type::getVoidTy(TheContext);
}

Value *IntegerExprAST::codeGen(CodeGenContext &context)
{
    return ConstantInt::get(Type::getInt64Ty(TheContext), value, true);
}

Value *DoubleExprAST::codeGen(CodeGenContext &context)
{
    return ConstantFP::get(TheContext, APFloat(value));
}

Value *IdentifierExprAST::codeGen(CodeGenContext &context)
{
    if (context.locals().find(name) == context.locals().end())
    {
        std::cerr << "undeclared variable " << name << std::endl;
        return nullptr;
    }
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value *AssignmentExprAST::codeGen(CodeGenContext &context)
{
    if (context.locals().find(lhs.name) == context.locals().end())
    {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        return nullptr;
    }
    return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value *BinaryOperatorExprAST::codeGen(CodeGenContext &context)
{
	Instruction::BinaryOps instr;
	switch (op) {
		case TADD: 	    instr = Instruction::Add; break;
		case TSUB: 	    instr = Instruction::Sub; break;
		case TMUL: 		instr = Instruction::Mul; break;
        case TDIV: 		instr = Instruction::SDiv; break;
        case TMOD:      instr = Instruction::URem; break;
        default:        return nullptr;
	}
	return BinaryOperator::Create(instr, lhs.codeGen(context), 
        rhs.codeGen(context), "", context.currentBlock());
}

Value *BlockExprAST::codeGen(CodeGenContext &context)
{
    StatementList::const_iterator it;
    Value *last = nullptr;
    for (it = statements.begin(); it != statements.end(); it++)
    {
        last = (**it).codeGen(context);
    }
    return last;
}

Value *ExprStmtAST::codeGen(CodeGenContext &context)
{
    return expression.codeGen(context);
}

Value *VariableDeclarationStmtAST::codeGen(CodeGenContext &context)
{
    AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
    context.locals()[id.name] = alloc;
    if (assignmentExpr != nullptr)
    {
        AssignmentExprAST assn(id, *assignmentExpr);
        assn.codeGen(context);
    }
    return alloc;
}

Value *PrintStmtAST::codeGen(CodeGenContext &context)
{
    Function *func = context.module->getFunction("print");
    std::vector<Value*> args;
    args.push_back(arg.codeGen(context));
    CallInst *call = CallInst::Create(func, makeArrayRef(args), "printCall", context.currentBlock());
    return call;
}