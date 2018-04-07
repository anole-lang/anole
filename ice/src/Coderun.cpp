#include "Coderun.h"
#include "Node.h"

Program::Program()
{
    top = nullptr;
    top = new Env(top);
}