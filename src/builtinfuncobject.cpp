#include <map>
#include "frame.hpp"
#include "noneobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
static map<string, Ptr<BuiltInFunctionObject>> &get_built_in_functions()
{
    static map<string, Ptr<BuiltInFunctionObject>> built_in_functions;
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
    const string &name, size_t args_num,
    function<ObjectPtr(vector<ObjectPtr>&)> func)
{
    get_built_in_functions()[name] = make_shared<BuiltInFunctionObject>(
        args_num, func
    );
}

void BuiltInFunctionObject::operator()(Ptr<Frame> frame)
{
    std::vector<ObjectPtr> args;
    for (size_t i = 0; i < args_num_; ++i)
    {
        args.push_back(frame->pop());
    }
    reverse(args.begin(), args.end());
    frame->push(func_(args));
}
}
