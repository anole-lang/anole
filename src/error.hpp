#pragma once

#include <string>
#include <exception>

namespace ice_language
{
class CompileError : public std::exception
{
  public:
    CompileError(std::string err)
      : err_(std::move(err)) {}

    CompileError &operator=(const CompileError &other) noexcept
    {
        err_ = other.err_;
        return *this;
    }

    const char *what() const noexcept override;

  private:
    std::string err_;
};

class RuntimeError : public std::exception
{
  public:
    RuntimeError(std::string err)
      : err_(std::move(err)) {}

    RuntimeError &operator=(const RuntimeError &other) noexcept
    {
        err_ = other.err_;
        return *this;
    }

    const char *what() const noexcept override;

  private:
    std::string err_;
};
}
