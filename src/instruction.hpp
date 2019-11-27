#pragma once

#include <memory>
#include <vector>
#include <string>

namespace ice_language
{
class Scope;

class Instruction
{
  public:
    virtual ~Instruction() = default;
    virtual void execute(Scope &scope) = 0;
};

class Pop : public Instruction
{
  public:
    void execute(Scope &scope) override;
};

class Push : public Instruction
{
  public:
    Push(std::shared_ptr<void> oprand)
      : oprand_(oprand) {}
    void execute(Scope &scope) override;

  private:
    std::shared_ptr<void> oprand_;
};

class Load : public Instruction
{
  public:
    Load(std::shared_ptr<std::string> name)
      : name_(name) {}
    void execute(Scope &scope) override;

  private:
    std::shared_ptr<std::string> name_;
};

class Store : public Instruction
{
  public:
    void execute(Scope &scope) override;
};

class Add: public Instruction
{
  public:
    void execute(Scope &scope) override;
};

class Sub: public Instruction
{
  public:
    void execute(Scope &scope) override;
};
}
