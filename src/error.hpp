#pragma once

#include <string>
#include <exception>
#include "context.hpp"

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
    RuntimeError(const std::string &err)
    {
        err_ = "\033[1mrunning at "
            + theCurrentContext->code()->from();
        auto &mapping = theCurrentContext->code()->mapping();
        if (mapping.count(theCurrentContext->pc()))
        {
            auto pos = mapping[theCurrentContext->pc()];
            err_ += ":" + to_string(pos.first)
                + ":" + to_string(pos.second)
                + ": \033[31merror: ";
        }
        err_ += err + "\033[0m";
    }

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
