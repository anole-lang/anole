#include "Coderun.h"
#include "Node.h"

void Env::put(std::string &name, IceObject *obj)
{
    objects[name] = obj;
}

IceObject *Env::getObject(std::string &name)
{
    Env *tmp = this;
    while (tmp != nullptr)
    {
        if (tmp->objects.find(name) != tmp->objects.end())
        {
            return tmp->objects[name];
        }
        else
        {
            tmp = tmp->prev;
        }
    }
    std::cout << "cannot find " << name << std::endl;
    exit(0);
    return nullptr;
}

IceObject *BlockExpr::runCode(Env *top)
{
    for (auto stmt : statements)
        stmt->runCode(top);
    return nullptr;
}

IceObject *IntegerExpr::runCode(Env *top)
{
    return new IceIntegerObject(value);
}

IceObject *IdentifierExpr::runCode(Env *top)
{
    return top->getObject(name);
}

IceObject *BinaryOperatorExpr::runCode(Env *top)
{
    IceObject *lobj = lhs->runCode(top);
    IceObject *robj = rhs->runCode(top);
    switch (op)
    {
    case Token::TOKEN::TADD:
        return lobj->add(robj);
    case Token::TOKEN::TSUB:
        return lobj->sub(robj);
    case Token::TOKEN::TMUL:
        return lobj->mul(robj);
    case Token::TOKEN::TDIV:
        return lobj->div(robj);
    case Token::TOKEN::TMOD:
        return lobj->mod(robj);
    default:
        return nullptr;
    }
}

IceObject *ExprStmt::runCode(Env *top)
{
    IceObject *obj = assignment->runCode(top);
    return obj;
}

IceObject *VariableDeclarationStmt::runCode(Env *top)
{
    IceObject *obj = assignment->runCode(top);
    top->put(id->name, obj);
    return nullptr;
}