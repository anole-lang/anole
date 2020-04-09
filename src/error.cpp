#include "error.hpp"
#include "context.hpp"

using namespace std;

namespace ice_language
{
CompileError::CompileError(string err)
  : err_(err) {}

CompileError
&CompileError::operator=(const CompileError &other) noexcept
{
    err_ = other.err_;
    return *this;
}

const char *CompileError::what() const noexcept
{
    return err_.c_str();
}

RuntimeError::RuntimeError(const string &err)
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

RuntimeError
&RuntimeError::operator=(const RuntimeError &other) noexcept
{
    err_ = other.err_;
    return *this;
}

const char *RuntimeError::what() const noexcept
{
    return err_.c_str();
}
}
