#pragma once

#include <vector>
#include <string>
#include <utility>
#include "helper.hpp"

namespace ice_language
{
class Scope;

class Instruction
{
  public:
    virtual ~Instruction() = 0;
    virtual void execute(Ptr<Scope> scope) = 0;
};

class Pop : public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Push : public Instruction
{
  public:
    Push(Ptr<void> oprand)
      : oprand_(oprand) {}
    void execute(Ptr<Scope> scope) override;

  private:
    Ptr<void> oprand_;
};

class Create : public Instruction
{
  public:
    Create(std::string name)
      : name_(std::move(name)) {}
    void execute(Ptr<Scope> scope) override;

  private:
    std::string name_;
};

class Load : public Instruction
{
  public:
    Load(std::string name)
      : name_(std::move(name)) {}
    void execute(Ptr<Scope> scope) override;

  private:
    std::string name_;
};

class Store : public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Call : public Instruction
{
  public:
    Call(std::size_t num) : num_(num) {}
    void execute(Ptr<Scope> scope) override;

  private:
    std::size_t num_;
};

class Neg : public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Add: public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Sub: public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Return : public Instruction
{
  public:
    void execute(Ptr<Scope> scope) override;
};

class Jump : public Instruction
{
  public:
    Jump(std::size_t ind)
      : ind_(ind) {}
    void execute(Ptr<Scope> scope) override;

  private:
    std::size_t ind_;
};

class JumpIfNot : public Instruction
{
  public:
    JumpIfNot(std::size_t ind)
      : ind_(ind) {}
    void execute(Ptr<Scope> scope) override;

  private:
    std::size_t ind_;
};
}
