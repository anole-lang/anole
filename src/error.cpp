#include "error.hpp"

namespace ice_language
{
const char *CompileError::what() const noexcept
{
    return err_.c_str();
}

const char *RuntimeError::what() const noexcept
{
    return err_.c_str();
}
}
