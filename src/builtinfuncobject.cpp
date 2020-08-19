#include "context.hpp"
#include "allocator.hpp"
#include "noneobject.hpp"
#include "builtinfuncobject.hpp"

#include <map>

using namespace std;

namespace anole
{
String BuiltInFunctionObject::to_str()
{
    return "<builtin-function>"s;
}

void BuiltInFunctionObject::call(Size n)
{
    func_(n);
    ++Context::current()->pc();
}

namespace
{
map<String, BuiltInFunctionObject *>
&get_built_in_functions()
{
    static map<String, BuiltInFunctionObject *> built_in_functions;
    return built_in_functions;
}
}

Object
*BuiltInFunctionObject::load_built_in_function(const String &name)
{
    if (get_built_in_functions().count(name))
    {
        return get_built_in_functions()[name];
    }
    return nullptr;
}

void BuiltInFunctionObject::register_built_in_function(
    const String &name, function<void(Size)> func)
{
    get_built_in_functions()[name]
        = Allocator<Object>::alloc<BuiltInFunctionObject>(func)
    ;
}
}
