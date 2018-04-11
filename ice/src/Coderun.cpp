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
        if(tmp->objects.find(name) != tmp->objects.end())
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
    return nullptr;
}

IceObject *IntegerExpr::runCode(Env *top)
{
    return new IceIntegerObject(value);
}

IceObject *IdentifierExpr::runCode(Env *top)
{
    return nullptr;
}

IceObject *ExprStmt::runCode(Env *top)
{
    IceObject* obj = assignment->runCode(top);
    return obj;
}

IceObject *VariableDeclarationStmt::runCode(Env *top)
{
    return nullptr;
}