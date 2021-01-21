#include "anole.hpp"

namespace anole
{
namespace info
{
String strong(const String &str)
{
    return "\033[1m" + str + "\033[0m";
}

String warning(const String &str)
{
    return "\033[31m" + str + "\033[0m";
}
} // namespace info

CompileError::CompileError(String err)
  : err_(err)
{
    while (theCurrContext && theCurrContext->pre_context())
    {
        theCurrContext = theCurrContext->pre_context();
    }
}

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

RuntimeError::RuntimeError(const String &err)
{
    auto &mapping = theCurrContext->code()->source_mapping();
    if (mapping.count(theCurrContext->pc()))
    {
        auto pos = mapping[theCurrContext->pc()];
        err_ = info::strong("  running at "
            + theCurrContext->code()->from()
            + ":" + std::to_string(pos.first)
            + ":" + std::to_string(pos.second) + ": "
        );
    }
    err_ += info::warning("error: ") + err;

    int trace_count = 0;
    while (theCurrContext->pre_context())
    {
        theCurrContext = theCurrContext->pre_context();

        if (trace_count < 66)
        {
            ++trace_count;

            auto &mapping = theCurrContext->code()->source_mapping();
            if (mapping.count(theCurrContext->pc()))
            {
                auto pos = mapping[theCurrContext->pc()];
                err_ = "  running at "
                    + theCurrContext->code()->from()
                    + ":" + std::to_string(pos.first)
                    + ":" + std::to_string(pos.second) + "\n"
                    + err_
                ;
            }
        }
    }

    err_ = info::strong("Traceback:\n") + err_;
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
