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
    /**
     * generate(String) will only return AnoleModule
    */
    static ModuleObject *generate(const String &name);
    /**
     * generate(path)
     *  return CppModule if path endwith .so,
     *  else return AnoleModule whose path should be not endwith .anole
    */
    static ModuleObject *generate(std::filesystem::path path);

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
    AnoleModuleObject(const std::filesystem::path &path);

    const SPtr<Scope> &scope() const;
    const SPtr<Code> &code() const;

  public:
    Address load_member(const String &name) override;

  private:
    void init(const std::filesystem::path &path);

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
};

class CppModuleObject : public ModuleObject
{
  public:
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
