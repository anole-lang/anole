#pragma once

#include <memory>
#include <vector>

namespace ice_language
{
class VM;

class Operation
{
  public:
    virtual ~Operation() = default;
    virtual void execute(VM &vm) = 0;
};

class Push : public Operation
{
  public:
    Push(std::vector<std::shared_ptr<void>> oprands)
      : oprands_(oprands) {}
    void execute(VM &vm) override;

  private:
    std::vector<std::shared_ptr<void>> oprands_;
};

class Add: public Operation
{
  public:
    void execute(VM &vm) override;
};

class Sub: public Operation
{
  public:
    void execute(VM &vm) override;
};
}
