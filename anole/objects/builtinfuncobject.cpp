#include "objects.hpp"

#include "../runtime/runtime.hpp"

#include <map>

namespace anole
{
namespace
{
std::map<String, BuiltInFunctionObject *>
&get_built_in_functions()
{
    static std::map<String, BuiltInFunctionObject *> built_in_functions;
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
    const String &name, std::function<void(Size)> func)
{
    /**
     * builtin functions won't be marked
     *  and always live until the program exits
    */
    get_built_in_functions()[name] = new BuiltInFunctionObject(func);
}

BuiltInFunctionObject::BuiltInFunctionObject(std::function<void(Size)> func, Object *bind) noexcept
  : Object(ObjectType::BuiltinFunc), func_(std::move(func)), bind_(bind)
{
    // ...
}

String BuiltInFunctionObject::to_str()
{
    return "<builtin-function>";
}

void BuiltInFunctionObject::call(Size n)
{
    func_(n);
    ++theCurrContext->pc();
}

void BuiltInFunctionObject::collect(std::function<void(Object *)> func)
{
    func(bind_);
}
}
