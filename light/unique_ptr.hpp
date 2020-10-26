#ifndef __LIGHT_UNIQUEPTR_HPP__
#define __LIGHT_UNIQUEPTR_HPP__

#include "type_traits.hpp"

namespace light
{
template<typename T>
struct default_delete
{
    constexpr default_delete() noexcept = default;

    void operator()(T *ptr) const
    {
        static_assert(!is_void_v<T>,
            "can't delete pointer to incomplete type")
        ;
        static_assert(sizeof(T) > 0,
            "can't delete pointer to incomplete type")
        ;
        delete ptr;
    }
};

template<typename T, typename D>
class unique_ptr
{
  public:
    using element_type = T;
};
} // namespace light

#endif
