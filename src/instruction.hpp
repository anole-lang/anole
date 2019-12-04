#pragma once

#include <vector>
#include <string>
#include <utility>
#include "helper.hpp"

/*

class Pop : public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Push : public Instruction
{
  public:
    Push(Ptr<void> oprand)
      : oprand_(oprand) {}
    void execute(Ptr<Frame> scope) override;

  private:
    Ptr<void> oprand_;
};

class Create : public Instruction
{
  public:
    Create(std::string name)
      : name_(std::move(name)) {}
    void execute(Ptr<Frame> scope) override;

  private:
    std::string name_;
};

class Load : public Instruction
{
  public:
    Load(std::string name)
      : name_(std::move(name)) {}
    void execute(Ptr<Frame> scope) override;

  private:
    std::string name_;
};

class Store : public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Call : public Instruction
{
  public:
    Call(std::size_t num) : num_(num) {}
    void execute(Ptr<Frame> scope) override;

  private:
    std::size_t num_;
};

class Neg : public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Add: public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Sub: public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Return : public Instruction
{
  public:
    void execute(Ptr<Frame> scope) override;
};

class Jump : public Instruction
{
  public:
    Jump(std::size_t ind)
      : ind_(ind) {}
    void execute(Ptr<Frame> scope) override;

  private:
    std::size_t ind_;
};

class JumpIfNot : public Instruction
{
  public:
    JumpIfNot(std::size_t ind)
      : ind_(ind) {}
    void execute(Ptr<Frame> scope) override;

  private:
    std::size_t ind_;
};
*/

namespace ice_language
{
class Frame;

enum class Op
{
    PlaceHolder,

    Pop,          // ok
    Push,         // ok

    Create,       // ok
    Load,         // ok
    Store,        // ok

    Neg,          // ok
    Add,          // ok
    Sub,          // ok

    Call,
    Return,       // ok
    Jump,         // ok
    JumpIfNot,
};

struct Instruction
{
    Op op;
    std::shared_ptr<void> oprand;
};
}
