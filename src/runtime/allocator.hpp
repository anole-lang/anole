#pragma once

#include "collector.hpp"

#include "../../light/type_traits.hpp"

#include <set>
#include <cassert>

namespace anole
{
/**
 * by allocator,
 *  we can allocate memories for variables
*/
template<typename T>
class Allocator
{
  public:
    static_assert(!light::is_void_v<T>);

    using Value = T;
    using Pointer = T *;

  public:
    static Allocator &allocator()
    {
        static Allocator<T> alctor;
        return alctor;
    }

    template<typename U = Value, typename ...Ts>
    static U *alloc(Ts &&...values)
    {
        return Allocator::allocator().template allocate<U>(std::forward<Ts>(values)...);
    }

    static void dealloc(Pointer ptr)
    {
        Allocator::allocator().deallocate(ptr);
    }

    static void dealloc(void *ptr)
    {
        dealloc(reinterpret_cast<Pointer>(ptr));
    }

  public:
    Allocator() = default;

    /**
     * allocate and construct
    */
    template<typename U = Value, typename ...Ts>
    U *allocate(Ts &&...values)
    {
        static_assert(std::is_convertible_v<U *, Pointer>);

        auto ptr = new U(std::forward<Ts>(values)...);

        Collector::collector().mark(Pointer(ptr));

        return ptr;
    }

    void deallocate(Pointer ptr)
    {
        delete ptr;
    }

    void deallocate(void *ptr)
    {
        deallocate(reinterpret_cast<Pointer>(ptr));
    }
};
} // namespace anole
