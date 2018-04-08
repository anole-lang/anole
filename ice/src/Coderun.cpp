#include "Coderun.h"
#include "Node.h"

void Env::put(std::string &name, void *value)
{
    values[name] = value;
}

void *Env::getValue(std::string &name)
{
    Env *tmp = this;
    while (tmp != nullptr)
    {
        if(tmp->values.find(name) != tmp->values.end())
        {
            return tmp->values[name];
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

std::string Env::getType(std::string &name)
{
    Env *tmp = this;
    while (tmp != nullptr)
    {
        if(tmp->types.find(name) != tmp->types.end())
        {
            return tmp->types[name];
        }
        else
        {
            tmp = tmp->prev;
        }
    }
    return NULL;
}

void *BlockExpr::runCode(Env *top)
{
    return nullptr;
}

void *IntegerExpr::runCode(Env *top)
{
    return nullptr;
}

void *IdentifierExpr::runCode(Env *top)
{
    return nullptr;
}

void *VariableDeclarationStmt::runCode(Env *top)
{
    return nullptr;
}