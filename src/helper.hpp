#pragma once

#include <memory>
#include <string>

namespace anole
{
template <typename T>
using SPtr = std::shared_ptr<T>;
template <typename T>
using Ptr = std::unique_ptr<T>;

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
