#pragma once

#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
class ModuleObject : public Object
{
  public:
    ModuleObject() = default;
    virtual ~ModuleObject() = 0;
    virtual Ptr<ObjectPtr> load_member(const std::string &name) = 0;

    bool good()
    {
        return good_;
    }

    static Ptr<ModuleObject> generate(const std::string &name);

  protected:
    bool good_;
};

class IceModuleObject : public ModuleObject
{
  public:
    IceModuleObject(const std::string &name);
    IceModuleObject(Ptr<Scope> scope)
      : scope_(scope) {}
    Ptr<ObjectPtr> load_member(const std::string &name) override;

    const Ptr<Scope> &scope() const
    {
        return scope_;
    }

  private:
    void init(const std::filesystem::path &path);

    Ptr<Scope> scope_;
};

class CppModuleObject
  : public ModuleObject,
    public std::enable_shared_from_this<CppModuleObject>
{
  public:
    CppModuleObject(const std::string &name);
    ~CppModuleObject();
    Ptr<ObjectPtr> load_member(const std::string &name) override;

    const std::vector<std::string> *names()
    {
        return names_;
    }

  private:
    void *handle_;
    std::vector<std::string> *names_;
};
}
