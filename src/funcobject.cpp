#include "funcobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace ice_language
{
string FunctionObject::to_str()
{
    return "<function>"s;
}

ObjectPtr FunctionObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr FunctionObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}

Ptr<ObjectPtr> FunctionObject::load_member(const string &name)
{
    return scope_->load_symbol(name);
}
}
