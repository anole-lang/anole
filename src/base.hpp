#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace anole
{
template<typename T>
using Ptr  = std::unique_ptr<T>;
template<typename T>
using SPtr = std::shared_ptr<T>;

using String = std::string;
using Size   = std::uint64_t;
}
