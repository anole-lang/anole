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
    pm.add(createPrintModulePass(outs()));
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

static Type *typeOf(const std::string &type)
{
    if (type == "int")
    {
        return Type::getInt64Ty(TheContext);
    }
    else if (type == "double")
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
    _type = context.types()[name];
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value *MethodCallExprAST::codeGen(CodeGenContext &context)
{
    Function *func = context.module->getFunction(name.c_str());
    _type = "int";
    if (func == NULL)
    {
        std::cerr << "no such function " << name << endl;
    }
    std::vector<Value *> args;
    for (auto &arg : arguments)
    {
        args.push_back(arg->codeGen(context));
    }
    CallInst *call = CallInst::Create(func, makeArrayRef(args), "", context.currentBlock());
    return call;
}

Value *BinaryOperatorExprAST::codeGen(CodeGenContext &context)
{
    Value *_lhs = lhs.codeGen(context);
    Value *_rhs = rhs.codeGen(context);
    this->_type = lhs._type;
    switch (op)
    {
    case TADD:
        return BinaryOperator::Create(Instruction::Add, _lhs, _rhs, "", context.currentBlock());
    case TSUB:
        return BinaryOperator::Create(Instruction::Sub, _lhs, _rhs, "", context.currentBlock());
    case TMUL:
        return BinaryOperator::Create(Instruction::Mul, _lhs, _rhs, "", context.currentBlock());
    case TDIV:
        return BinaryOperator::Create(Instruction::SDiv, _lhs, _rhs, "", context.currentBlock());
    case TMOD:
        return BinaryOperator::Create(Instruction::SRem, _lhs, _rhs, "", context.currentBlock());

    case TCEQ:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, _lhs, _rhs, "", context.currentBlock());
    case TCNE:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_NE, _lhs, _rhs, "", context.currentBlock());
    case TCLT:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLT, _lhs, _rhs, "", context.currentBlock());
    case TCLE:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLE, _lhs, _rhs, "", context.currentBlock());
    case TCGT:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGT, _lhs, _rhs, "", context.currentBlock());
    case TCGE:
        return CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGE, _lhs, _rhs, "", context.currentBlock());
    default:
        return nullptr;
    }
}

Value *BlockExprAST::codeGen(CodeGenContext &context)
{
    Value *last = nullptr;
    for (auto &it: statements)
    {
        last = (*it).codeGen(context);
    }
    return last;   
}

Value *ExprStmtAST::codeGen(CodeGenContext &context)
{
    return expression.codeGen(context);
}

Value *VariableDeclarationStmtAST::codeGen(CodeGenContext &context)
{
    if (assignmentExpr == nullptr)
        return nullptr;
    Value *_rhs = assignmentExpr->codeGen(context);
    id._type = assignmentExpr->_type;
    context.types()[id.name] = id._type;
    AllocaInst *alloc = new AllocaInst(typeOf(id._type), id.name.c_str(), context.currentBlock());
    context.locals()[id.name] = alloc;
    return new StoreInst(_rhs, alloc, false, context.currentBlock());
}

Value *FunctionDeclarationStmtAST::codeGen(CodeGenContext &context)
{
    vector<Type *> argTypes;
    for (auto &var : arguments)
    {
        argTypes.push_back(Type::getInt64Ty(TheContext));
    }
    FunctionType *ftype = FunctionType::get(Type::getInt64Ty(TheContext), makeArrayRef(argTypes), false);
    Function *func = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
    BasicBlock *bblock = BasicBlock::Create(TheContext, "entry", func, 0);

    context.pushBlock(bblock);

    Function::arg_iterator argsValues = func->arg_begin();
    Value *argumentValue;
    for (auto &var : arguments)
    {
        AllocaInst *alloc = new AllocaInst(Type::getInt64Ty(TheContext), var->name.c_str(), context.currentBlock());
        context.locals()[var->name] = alloc;
        context.types()[var->name] = "int";

        argumentValue = &*argsValues++;
        argumentValue->setName(var->name.c_str());
        StoreInst *inst = new StoreInst(argumentValue, context.locals()[var->name], false, bblock);
    }

    block.codeGen(context);
    ReturnInst::Create(TheContext, context.getCurrentReturnValue(), bblock);

    context.popBlock();
    return func;
}

Value *ReturnStmtAST::codeGen(CodeGenContext &context)
{
    Value *returnValue = expression.codeGen(context);
    context.setCurrentReturnValue(returnValue);
    return returnValue;
}

Value *IfElseStmtAST::codeGen(CodeGenContext &context)
{
    Value *CondV = cond.codeGen(context);
    if(!CondV) return nullptr;

    Value *_zero = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, ConstantInt::get(Type::getInt64Ty(TheContext), 1, true), ConstantInt::get(Type::getInt64Ty(TheContext), 0, true), "", context.currentBlock());
    CondV = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, CondV, _zero, "ifcond", context.currentBlock());

    IRBuilder<> Builder = IRBuilder<>(context.currentBlock());
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    BasicBlock *thenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *elseBB = BasicBlock::Create(TheContext, "else", TheFunction);
    BasicBlock *mergeBB = BasicBlock::Create(TheContext, "ifcont", TheFunction);

    Builder.CreateCondBr(CondV, thenBB, elseBB);
    //BranchInst::Create(thenBB, elseBB, CondV, context.currentBlock());

    Builder.SetInsertPoint(thenBB);
    context.pushBlock(thenBB);
    Value *thenV = blockTrue.codeGen(context);
    Builder.CreateBr(mergeBB);
    context.popBlock();

    Builder.SetInsertPoint(elseBB);
    context.pushBlock(elseBB);
    Value *elseV = blockFalse.codeGen(context);
    Builder.CreateBr(mergeBB);
    context.popBlock();

    Builder.SetInsertPoint(mergeBB);
    context.pushBlock(mergeBB);
    //PHINode *PN = Builder.CreatePHI(Type::getVoidTy(TheContext), 2, "iftmp");
    //PN->addIncoming(thenV, thenBB);
    //PN->addIncoming(elseV, elseBB);
    //return PN;
    
    return nullptr;
}