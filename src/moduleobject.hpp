#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class ModuleObject : public Object
{
  public:
    ModuleObject()
      : Object(ObjectType::Module) {}
    virtual ~ModuleObject() = 0;
    virtual Address load_member(const std::string &name) = 0;

    bool good()
    {
        return good_;
    }

    static SPtr<ModuleObject> generate(const std::string &name);
    static SPtr<ModuleObject> generate(const std::filesystem::path &path);

  protected:
    bool good_;
};

class AnoleModuleObject : public ModuleObject
{
  public:
    AnoleModuleObject(const std::string &name);
    AnoleModuleObject(const std::filesystem::path &path);
    Address load_member(const std::string &name) override;

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

class CppModuleObject
  : public ModuleObject,
    public std::enable_shared_from_this<CppModuleObject>
{
  public:
    CppModuleObject(const std::string &name);
    CppModuleObject(const std::filesystem::path &path);
    ~CppModuleObject();
    Address load_member(const std::string &name) override;

    const std::vector<std::string> *names()
    {
        return names_;
    }

  private:
    void *handle_;
    std::vector<std::string> *names_;
};
}
