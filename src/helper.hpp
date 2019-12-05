#pragma once

#include <memory>

namespace ice_language
{
template <typename T>
using Ptr = std::shared_ptr<T>;
using VoidPtr = Ptr<void>;
}
