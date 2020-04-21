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

void ContObject::call(size_t)
{
    auto retval = theCurrentContext->pop();
    theCurrentContext = make_shared<Context>(resume_);
    theCurrentContext->push(retval);
    ++theCurrentContext->pc();
}

void ContObject::call_tail(size_t)
{
    auto retval = theCurrentContext->pop();
    theCurrentContext = make_shared<Context>(resume_);
    theCurrentContext->push(retval);
    ++theCurrentContext->pc();
}
}
