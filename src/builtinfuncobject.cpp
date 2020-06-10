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

void BuiltInFunctionObject::call()
{
    theCurrentContext->call_anchors().pop();
    func_();
    ++theCurrentContext->pc();
}

void BuiltInFunctionObject::call_tail()
{
    theCurrentContext->call_anchors().pop();
    func_();
    ++theCurrentContext->pc();
}

static map<string, SPtr<BuiltInFunctionObject>> &get_built_in_functions()
{
    static map<string, SPtr<BuiltInFunctionObject>> built_in_functions;
    return built_in_functions;
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
    const string &name, function<void()> func)
{
    get_built_in_functions()[name] = make_shared<BuiltInFunctionObject>(func);
}
}
