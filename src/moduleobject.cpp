#include <dlfcn.h>
#include <fstream>
#include "parser.hpp"
#include "context.hpp"
#include "moduleobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;
using namespace filesystem;
using namespace chrono_literals;

namespace anole
{
ModuleObject::~ModuleObject() = default;

SPtr<ModuleObject> ModuleObject::generate(const string &name)
{
    SPtr<ModuleObject> mod = make_shared<AnoleModuleObject>(name);
    if (!mod->good())
    {
        mod = make_shared<CppModuleObject>(name);
    }

    return mod;
}

AnoleModuleObject::AnoleModuleObject(const string &name)
{
    good_ = true;
    if (is_regular_file(theCurrentContext->current_path() / (name + ".anole")))
    {
        init(theCurrentContext->current_path() / (name + ".anole"));
    }
    else if (is_directory(theCurrentContext->current_path() / name))
    {
        init(theCurrentContext->current_path() / name / "__init__.anole");
    }
    else if (is_regular_file(filesystem::path("/usr/local/lib/anole") / (name + ".anole")))
    {
        init(filesystem::path("/usr/local/lib/anole") / (name + ".anole"));
    }
    else if (is_directory(filesystem::path("/usr/local/lib/anole") / name))
    {
        init(filesystem::path("/usr/local/lib/anole") / name / "__init__.anole");
    }
    else
    {
        good_ = false;
    }
}

Address AnoleModuleObject::load_member(const string &name)
{
    if (scope_->symbols().count(name))
    {
        return scope_->load_symbol(name);
    }
    return Object::load_member(name);
}

void AnoleModuleObject::init(const filesystem::path &path)
{
    auto dir = path.parent_path();
    auto ir_path = path.string() + ".ir";

    code_ = make_shared<Code>(path.filename().string());
    auto origin = theCurrentContext;
    theCurrentContext = make_shared<Context>(code_, dir);
    theCurrentContext->pre_context() = origin;

    if (is_regular_file(ir_path)
        and last_write_time(ir_path) >= last_write_time(path))
    {
        code_->unserialize(ir_path);
        Context::execute();
    }
    else
    {
        ifstream fin{path};
        if (!fin.good())
        {
            throw RuntimeError("cannot open file " + path.string());
        }

        Parser parser{fin, path};

        while (auto stmt = parser.gen_statement())
        {
            stmt->codegen(*code_);
            Context::execute();
        }

        code_->serialize(ir_path);
    }
    #ifdef _DEBUG
    auto rd_path = path;
    rd_path += ".rd";
    code_->print(rd_path);
    #endif

    scope_ = theCurrentContext->scope();
    theCurrentContext = origin;
}

CppModuleObject::CppModuleObject(const string &name)
{
    auto path = theCurrentContext->current_path() / (name + ".so");
    handle_ = dlopen(path.c_str(), RTLD_NOW);
    if (!handle_)
    {
        auto path = "/usr/local/lib/anole/" + name + ".so";
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

Address CppModuleObject::load_member(const string &name)
{
    using FuncType = void (*)();
    auto func = reinterpret_cast<FuncType>(dlsym(handle_, name.c_str()));
    if (!func)
    {
        throw RuntimeError(dlerror());
    }
    auto result = make_shared<BuiltInFunctionObject>(
        [mod = shared_from_this(), func] { func(); });
    return make_shared<ObjectPtr>(result);
}
}
