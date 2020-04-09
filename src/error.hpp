#pragma once

#include <string>
#include <exception>

namespace ice_language
{
class CompileError : public std::exception
{
  public:
    CompileError(std::string err);
    CompileError &operator=(const CompileError &other) noexcept;

    const char *what() const noexcept override;

  private:
    std::string err_;
};

class RuntimeError : public std::exception
{
  public:
    RuntimeError(const std::string &err);
    RuntimeError &operator=(const RuntimeError &other) noexcept;

    const char *what() const noexcept override;

  private:
    std::string err_;
};
}
