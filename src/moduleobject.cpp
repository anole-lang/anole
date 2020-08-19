#include "parser.hpp"
#include "context.hpp"
#include "moduleobject.hpp"
#include "builtinfuncobject.hpp"

#ifdef __linux__
#include <dlfcn.h>
#else
#error "only support linux"
#endif

#include <fstream>

using namespace std;
namespace fs = filesystem;

namespace anole
{
ModuleObject::~ModuleObject() = default;

ModuleObject *ModuleObject::generate(const String &name)
{
    ModuleObject *mod = Allocator<Object>::alloc<AnoleModuleObject>(name);
    if (!mod->good())
    {
        mod = Allocator<Object>::alloc<CppModuleObject>(name);
    }
    return mod;
}

ModuleObject *ModuleObject::generate(const fs::path &path)
{
    ModuleObject *mod;
    if (path.extension() == ".so")
    {
        mod = Allocator<Object>::alloc<CppModuleObject>(path);
    }
    else
    {
        mod = Allocator<Object>::alloc<AnoleModuleObject>(path);
    }
    return mod;
}

AnoleModuleObject::AnoleModuleObject(const String &name)
  : ModuleObject(ObjectType::AnoleModule)
{
    good_ = true;

    /**
     * use module-name will look up the module in current directory firstly
     *  and then look up in "/usr/local/lib/anole/" if not find
    */

    auto cpath = Context::current()->current_path();
    if (fs::is_regular_file(cpath / (name + ".anole")))
    {
        init(cpath / (name + ".anole"));
    }
    else if (fs::is_directory(cpath / name))
    {
        init(cpath / name / "__init__.anole");
    }
    else if (fs::is_regular_file(fs::path("/usr/local/lib/anole") / (name + ".anole")))
    {
        init(fs::path("/usr/local/lib/anole") / (name + ".anole"));
    }
    else if (fs::is_directory(fs::path("/usr/local/lib/anole") / name))
    {
        init(fs::path("/usr/local/lib/anole") / name / "__init__.anole");
    }
    else
    {
        good_ = false;
    }
}

AnoleModuleObject::AnoleModuleObject(const fs::path &path)
  : ModuleObject(ObjectType::AnoleModule)
{
    good_ = true;

    /**
     * support "/path/to/mod.anole"
     *  and "/path/to/mod" with "/path/to/mod/__init__.anole"
    */
    if (path.extension() == ".anole")
    {
        init(path);
    }
    else if (fs::is_directory(path))
    {
        init(path / "__init__.anole");
    }
    else
    {
        good_ = false;
    }
}

Address AnoleModuleObject::load_member(const String &name)
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
    auto origin = Context::current();
    Context::current() = Allocator<Context>::alloc(code_, dir);
    Context::current()->pre_context() = origin;

    if (fs::is_regular_file(ir_path)
        && fs::last_write_time(ir_path) >= fs::last_write_time(path)
        && code_->unserialize(ir_path))
    {
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

    scope_ = Context::current()->scope();
    Context::current() = origin;
}

CppModuleObject::CppModuleObject(const String &name)
  : ModuleObject(ObjectType::CppModule)
{
    auto path = Context::current()->current_path() / (name + ".so");
    handle_ = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_DEEPBIND);
    if (!handle_)
    {
        auto path = "/usr/local/lib/anole/" + name + ".so";
        handle_ = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_DEEPBIND);
    }
    good_ = handle_;
    names_ = good_
        ? reinterpret_cast<decltype(names_)>(dlsym(handle_, "_FUNCTIONS"))
        : nullptr
    ;
}

CppModuleObject::CppModuleObject(const fs::path &path)
  : ModuleObject(ObjectType::CppModule)
{
    handle_ = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_DEEPBIND);
    good_ = handle_;
    names_ = good_
        ? reinterpret_cast<decltype(names_)>(dlsym(handle_, "_FUNCTIONS"))
        : nullptr
    ;
}

CppModuleObject::~CppModuleObject()
{
    if (good_)
    {
        dlclose(handle_);
    }
}

Address CppModuleObject::load_member(const String &name)
{
    using FuncType = void (*)(Size);

    auto func = reinterpret_cast<FuncType>(dlsym(handle_, name.c_str()));
    if (!func)
    {
        throw RuntimeError(dlerror());
    }

    auto result = Allocator<Object>::alloc<BuiltInFunctionObject>(
        [func](Size n) { func(n); }, this
    );

    return Allocator<Variable>::alloc(result);
}
}
