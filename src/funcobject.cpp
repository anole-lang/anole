#include "context.hpp"
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

SPtr<ObjectPtr> FunctionObject::load_member(const string &name)
{
    auto ptr = scope_->load_symbol(name);
    if (!*ptr)
    {
        Context::add_not_defined_symbol(name, ptr);
    }
    return ptr;
}
}
