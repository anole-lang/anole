#include <dlfcn.h>
#include <fstream>
#include "parser.hpp"
#include "context.hpp"
#include "moduleobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
ModuleObject::~ModuleObject() = default;

IceModuleObject::IceModuleObject(const string &name)
{
    ifstream fin{name + ".ice"};
    if (fin.good())
    {
        auto code = make_shared<Code>();
        Parser(fin).gen_statements()->codegen(*code);
        auto origin = theCurrentContext;
        theCurrentContext = make_shared<Context>(code);
        Context::execute();
        scope_ = theCurrentContext->scope();
        theCurrentContext = origin;
    }
    else
    {
        good_ = false;
    }
}

Ptr<ObjectPtr> IceModuleObject::load_member(const string &name)
{
    if (scope_->symbols().count(name))
    {
        return scope_->load_symbol(name);
    }
    return Object::load_member(name);
}

CppModuleObject::CppModuleObject(const string &name)
{
    auto path = "./" + name + ".so";
    handle_ = dlopen(path.c_str(), RTLD_NOW);
    good_ = handle_;
    names_ = good_
        ? reinterpret_cast<decltype(names_)>(dlsym(handle_, "_FUNCTIONS"))
        : nullptr;
}

CppModuleObject::~CppModuleObject()
{
    if (good_)
    {
        dlclose(handle_);
    }
}

Ptr<ObjectPtr> CppModuleObject::load_member(const string &name)
{
    using FuncType = void (*)();
    auto func = reinterpret_cast<FuncType>(dlsym(handle_, name.c_str()));
    if (!func)
    {
        throw runtime_error(dlerror());
    }
    auto result = make_shared<BuiltInFunctionObject>(
        [mod = shared_from_this(), func] { func(); });
    return make_shared<ObjectPtr>(result);
}
}
