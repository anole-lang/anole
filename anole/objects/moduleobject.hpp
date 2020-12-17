#ifndef __ANOLE_OBJECTS_MODULE_HPP__
#define __ANOLE_OBJECTS_MODULE_HPP__

#include "object.hpp"

#include <filesystem>

namespace anole
{
class Code;

class ModuleObject : public Object
{
  public:
    static ModuleObject *generate(const String &name);
    static ModuleObject *generate(const std::filesystem::path &path);

  public:
    ModuleObject(ObjectType type) noexcept;
    virtual ~ModuleObject() = 0;

    bool good();

  public:
    virtual Address load_member(const String &name) = 0;

  protected:
    bool good_;
};

class AnoleModuleObject : public ModuleObject
{
  public:
    AnoleModuleObject(const String &name);
    AnoleModuleObject(const std::filesystem::path &path);

    const SPtr<Scope> &scope() const;
    const SPtr<Code> &code() const;

  public:
    Address load_member(const String &name) override;

    void collect(std::function<void(Scope *)>) override;

  private:
    void init(const std::filesystem::path &path);

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
};

class CppModuleObject : public ModuleObject
{
  public:
    CppModuleObject(const String &name);
    CppModuleObject(const std::filesystem::path &path);
    ~CppModuleObject();

    const std::vector<String> *names() const;

  public:
    Address load_member(const String &name) override;

  private:
    void *handle_;
    std::vector<String> *names_;
};
}

#endif
