#pragma once

#include <memory>
#include <string>

namespace ice_language
{
template <typename T>
using Ptr = std::shared_ptr<T>;

namespace info
{
inline std::string strong(const std::string &str)
{
    return "\033[1m" + str + "\033[0m";
}

inline std::string warning(const std::string &str)
{
    return "\033[31m" + str + "\033[0m";
}
}
}
