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

void ContObject::call(size_t n)
{
    if (n != 1)
    {
        throw RuntimeError("continuation need a argument");
    }
    auto retval = theCurrentContext->pop();
    theCurrentContext = make_shared<Context>(resume_);
    theCurrentContext->push(retval);
    ++theCurrentContext->pc();
}
}
