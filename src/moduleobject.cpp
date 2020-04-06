#include <dlfcn.h>
#include <fstream>
#include "parser.hpp"
#include "context.hpp"
#include "moduleobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;
using namespace filesystem;

namespace ice_language
{
ModuleObject::~ModuleObject() = default;

Ptr<ModuleObject> ModuleObject::generate(const string &name)
{
    Ptr<ModuleObject> mod = make_shared<IceModuleObject>(name);
    if (!mod->good())
    {
        mod = make_shared<CppModuleObject>(name);
    }

    return mod;
}

IceModuleObject::IceModuleObject(const string &name)
{
    good_ = true;
    if (is_directory(theCurrentContext->current_path() / name))
    {
        init(theCurrentContext->current_path() / name / "__init__.ice");
    }
    else if (is_regular_file(theCurrentContext->current_path() / (name + ".ice")))
    {
        init(theCurrentContext->current_path() / (name + ".ice"));
    }
    else if (is_directory(filesystem::path("/usr/local/lib/ice-lang") / name))
    {
        init(filesystem::path("/usr/local/lib/ice-lang") / name / "__init__.ice");
    }
    else if (is_regular_file(filesystem::path("/usr/local/lib/ice-lang") / (name + ".ice")))
    {
        init(filesystem::path("/usr/local/lib/ice-lang") / (name + ".ice"));
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

void IceModuleObject::init(const filesystem::path &path)
{
    auto dir = path.parent_path();
    ifstream fin{path};
    if (!fin.good())
    {
        throw runtime_error("cannot open file " + path.string());
    }
    auto code = make_shared<Code>();
    Parser(fin).gen_statements()->codegen(*code);
    auto origin = theCurrentContext;
    theCurrentContext = make_shared<Context>(code, dir);
    Context::execute();
    scope_ = theCurrentContext->scope();
    theCurrentContext = origin;
}

CppModuleObject::CppModuleObject(const string &name)
{
    auto path = theCurrentContext->current_path() / (name + ".so");
    handle_ = dlopen(path.c_str(), RTLD_NOW);
    if (!handle_)
    {
        auto path = "/usr/local/lib/ice-lang/" + name + ".so";
        handle_ = dlopen(path.c_str(), RTLD_NOW);
    }
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
