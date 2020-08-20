#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class ModuleObject : public Object
{
  public:
    ModuleObject(ObjectType type) : Object(type) {}
    virtual ~ModuleObject() = 0;
    virtual Address load_member(const String &name) = 0;

    bool good()
    {
        return good_;
    }

    static SPtr<ModuleObject> generate(const String &name);
    static SPtr<ModuleObject> generate(const std::filesystem::path &path);

  protected:
    bool good_;
};

class AnoleModuleObject : public ModuleObject
{
  public:
    friend class Collector;

    AnoleModuleObject(const String &name);
    AnoleModuleObject(const std::filesystem::path &path);
    Address load_member(const String &name) override;

    const SPtr<Scope> &scope() const
    {
        return scope_;
    }
    const SPtr<Code> &code() const
    {
        return code_;
    }

  private:
    void init(const std::filesystem::path &path);

    SPtr<Scope> scope_;
    SPtr<Code> code_;
};

class CppModuleObject : public ModuleObject, public std::enable_shared_from_this<CppModuleObject>
{
  public:
    CppModuleObject(const String &name);
    CppModuleObject(const std::filesystem::path &path);
    ~CppModuleObject();
    Address load_member(const String &name) override;

    const std::vector<String> *names()
    {
        return names_;
    }

  private:
    void *handle_;
    std::vector<String> *names_;
};
}
