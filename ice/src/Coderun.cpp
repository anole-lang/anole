#include "Coderun.h"
#include "Node.h"

void Env::put(std::string &name, Object *obj)
{
    objects[name] = obj;
}

Object *Env::getObject(std::string &name)
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

Object *BlockExpr::runCode(Env *top)
{
    return nullptr;
}

Object *IntegerExpr::runCode(Env *top)
{
    static const int i = value;
    void *val = (void *)&i;
    return new Object(val, "int");
}

Object *IdentifierExpr::runCode(Env *top)
{
    return nullptr;
}

Object *ExprStmt::runCode(Env *top)
{
    return nullptr;
}

Object *VariableDeclarationStmt::runCode(Env *top)
{
    return nullptr;
}