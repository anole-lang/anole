#include "base.hpp"
#include "error.hpp"
#include "context.hpp"

using namespace std;

namespace anole
{
CompileError::CompileError(String err)
  : err_(err)
{
    while (Context::current() && Context::current()->pre_context())
    {
        Context::current() = Context::current()->pre_context();
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
    auto &mapping = Context::current()->code()->mapping();
    if (mapping.count(Context::current()->pc()))
    {
        auto pos = mapping[Context::current()->pc()];
        err_ = info::strong("  running at "
            + Context::current()->code()->from()
            + ":" + to_string(pos.first)
            + ":" + to_string(pos.second) + ": "
        );
    }
    err_ += info::warning("error: ") + err;

    int trace_count = 0;
    while (Context::current()->pre_context())
    {
        Context::current() = Context::current()->pre_context();

        if (trace_count < 66)
        {
            ++trace_count;

            auto &mapping = Context::current()->code()->mapping();
            if (mapping.count(Context::current()->pc()))
            {
                auto pos = mapping[Context::current()->pc()];
                err_ = "  running at "
                    + Context::current()->code()->from()
                    + ":" + to_string(pos.first)
                    + ":" + to_string(pos.second) + "\n"
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
