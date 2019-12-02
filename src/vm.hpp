#pragma once

#include <stack>
#include <memory>
#include "scope.hpp"

namespace ice_language
{
class VM
{
  public:
    VM() : scope_(std::make_shared<Scope>()) {}
    void execute_code(Code &code);

  private:
    Ptr<Scope> scope_;
};
} // namespace ice_language
