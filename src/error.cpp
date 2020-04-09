#include "error.hpp"
#include "helper.hpp"
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
    auto &mapping = theCurrentContext->code()->mapping();
    if (mapping.count(theCurrentContext->pc()))
    {
        auto pos = mapping[theCurrentContext->pc()];
        err_ = info::strong("  running at "
            + theCurrentContext->code()->from()
            + ":" + to_string(pos.first)
            + ":" + to_string(pos.second) + ": ");
    }
    err_ += info::warning("error: ") + err;

    int trace_count = 0;
    while (theCurrentContext->pre_context())
    {
        theCurrentContext = theCurrentContext->pre_context();

        if (trace_count < 66)
        {
            auto &mapping = theCurrentContext->code()->mapping();
            if (mapping.count(theCurrentContext->pc()))
            {
                auto pos = mapping[theCurrentContext->pc()];
                err_ = "  running at "
                    + theCurrentContext->code()->from()
                    + ":" + to_string(pos.first)
                    + ":" + to_string(pos.second) + "\n"
                    + err_;
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
