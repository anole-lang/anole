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
};

class IceModuleObject : public ModuleObject
{
  public:
    IceModuleObject(Ptr<Scope> scope)
      : scope_(scope) {}
    Ptr<ObjectPtr> load_member(const std::string &name) override;

  private:
    Ptr<Scope> scope_;
};

class CppModuleObject : public ModuleObject
{
  public:
    CppModuleObject(const std::string &path);
    ~CppModuleObject();
    Ptr<ObjectPtr> load_member(const std::string &name) override;

    bool good()
    {
        return good_;
    }

  private:
    bool good_;
    void *handle_;
};
}
