#pragma once

#include "type_traits.hpp"

namespace light
{
template<typename T, typename D>
class unique_ptr
{
  public:
    using element_type = T;
};
} // namespace light
