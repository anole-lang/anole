#include <map>
#include "context.hpp"
#include "noneobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace anole
{
string BuiltInFunctionObject::to_str()
{
    return "<builtin-function>"s;
}

void BuiltInFunctionObject::call(size_t n)
{
    func_(n);
    ++theCurrentContext->pc();
}

void BuiltInFunctionObject::call_tail(size_t n)
{
    func_(n);
    ++theCurrentContext->pc();
}

namespace
{
map<string, SPtr<BuiltInFunctionObject>> &get_built_in_functions()
{
    static map<string, SPtr<BuiltInFunctionObject>> built_in_functions;
    return built_in_functions;
}
}

ObjectPtr
BuiltInFunctionObject::load_built_in_function(const string &name)
{
    if (get_built_in_functions().count(name))
    {
        return get_built_in_functions()[name];
    }
    return nullptr;
}

void BuiltInFunctionObject::register_built_in_function(
    const string &name, function<void(std::size_t)> func)
{
    get_built_in_functions()[name] = make_shared<BuiltInFunctionObject>(func);
}
}
