#pragma once

#include <memory>
#include <vector>
#include <string>
#include "helper.hpp"

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
    Push(Ptr<void> oprand)
      : oprand_(oprand) {}
    void execute(Scope &scope) override;

  private:
    Ptr<void> oprand_;
};

class Load : public Instruction
{
  public:
    Load(Ptr<std::string> name)
      : name_(name) {}
    void execute(Scope &scope) override;

  private:
    Ptr<std::string> name_;
};

class Store : public Instruction
{
  public:
    void execute(Scope &scope) override;
};

class Call : public Instruction
{
  public:
    Call(Ptr<std::size_t> num) : num_(num) {}
    void execute(Scope &scope) override;

  private:
    Ptr<std::size_t> num_;
};

class Neg : public Instruction
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
