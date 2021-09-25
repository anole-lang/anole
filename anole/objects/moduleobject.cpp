#include "objects.hpp"

#include "../runtime/runtime.hpp"
#include "../compiler/compiler.hpp"

#ifdef __linux__
    #include <dlfcn.h>
#else
    #error only support linux
#endif

#include <fstream>

#ifdef _DEBUG
    #include <iostream>
#endif

namespace fs = std::filesystem;

namespace anole
{
namespace // local
{
/**
 * TODO:
 *  add more information about loaded modules
 *   such as the create-time
*/
std::map<fs::path, ModuleObject *> localLoadedModules;
}

ModuleObject *ModuleObject::generate(const String &name)
{
    fs::path path;

    // assume absolute path
    const auto &code_path = theCurrContext->code_path();
    if (fs::is_regular_file(code_path / (name + ".anole")))
    {
        path = code_path / (name + ".anole");
    }
    else if (fs::is_directory(code_path / name))
    {
        path = code_path / name / "__init__.anole";
    }
    else if (fs::is_regular_file(fs::path("/usr/local/lib/anole") / (name + ".anole")))
    {
        path = fs::path("/usr/local/lib/anole") / (name + ".anole");
    }
    else if (fs::is_directory(fs::path("/usr/local/lib/anole") / name))
    {
        path = fs::path("/usr/local/lib/anole") / name / "__init__.anole";
    }
    else
    {
        throw RuntimeError("no module named " + name);
    }

    return generate(path);
}

ModuleObject *ModuleObject::generate(fs::path path)
{
    if (path.is_relative())
    {
        path = theCurrContext->code_path() / path;
    }

    /**
     * NOTE:
     *  only standardize the path of code here
    */
    path = path.lexically_normal();

    auto find = localLoadedModules.find(path);
    if (find != localLoadedModules.end())
    {
        return find->second;
    }

    ModuleObject *mod;
    if (path.extension() == ".so")
    {
        mod = new CppModuleObject(path);
    }
    else
    {
        mod = new AnoleModuleObject(path);
    }

    if (!mod->good())
    {
        delete mod;
        mod = nullptr;
    }
    else
    {
        localLoadedModules[path] = mod;
    }
    return mod;
}

ModuleObject::ModuleObject(ObjectType type) noexcept
  : Object(type), good_(false)
{
    // ...
}

ModuleObject::~ModuleObject() = default;

bool ModuleObject::good()
{
    return good_;
}

// assume absolute path
AnoleModuleObject::AnoleModuleObject(const fs::path &path)
  : ModuleObject(ObjectType::AnoleModule)
{
    auto dir = path.parent_path();
    auto ir_path = path.string() + ".ir";

    code_ = std::make_shared<Code>(path.filename().string(), dir);
    auto origin = theCurrContext;
    theCurrContext = std::make_shared<Context>(code_);
    theCurrContext->pre_context() = origin;

    if (fs::is_regular_file(ir_path)
        && fs::last_write_time(ir_path) >= fs::last_write_time(path)
        && code_->unserialize(ir_path))
    {
        Context::execute();
    }
    else
    {
        std::ifstream fin{path};
        if (!fin.good())
        {
            throw RuntimeError("cannot open file " + path.string());
        }

        Parser parser{fin, path};

        while (auto stmt = parser.gen_statement())
        {
            stmt->codegen(*code_);

          #ifdef _DEBUG
            code_->print(std::cerr);
          #endif

            Context::execute();
        }

        code_->serialize(ir_path);
    }

  #ifdef _DEBUG
    auto rd_path = path;
    rd_path += ".rd";
    code_->print(rd_path);
  #endif

    scope_ = theCurrContext->scope();
    theCurrContext = origin;

    good_ = true;
}

const SPtr<Scope> &AnoleModuleObject::scope() const
{
    return scope_;
}

const SPtr<Code> &AnoleModuleObject::code() const
{
    return code_;
}

Address AnoleModuleObject::load_member(const String &name)
{
    if (scope_->symbols().count(name))
    {
        return scope_->load_symbol(name);
    }
    return Object::load_member(name);
}

// assume absolute path
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

const std::vector<String> *CppModuleObject::names() const
{
    return names_;
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

    return std::make_shared<Variable>(result);
}
}
