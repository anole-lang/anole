#include "contobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
ObjectPtr ContObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr ContObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}

void ContObject::call()
{
    theCurrentContext->call_anchors().pop();
    auto retval = theCurrentContext->pop();
    theCurrentContext = make_shared<Context>(resume_);
    theCurrentContext->push(retval);
    ++theCurrentContext->pc();
}

void ContObject::call_tail()
{
    theCurrentContext->call_anchors().pop();
    auto retval = theCurrentContext->pop();
    theCurrentContext = make_shared<Context>(resume_);
    theCurrentContext->push(retval);
    ++theCurrentContext->pc();
}
}
