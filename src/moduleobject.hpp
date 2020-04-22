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

  protected:
    bool good_;
};

class AnoleModuleObject : public ModuleObject
{
  public:
    AnoleModuleObject(const std::string &name);
    AnoleModuleObject(SPtr<Scope> scope)
      : scope_(scope) {}
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
