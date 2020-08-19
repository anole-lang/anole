#pragma once

#include <set>
#include <cassert>

#include "collector.hpp"
#include "../light/type_traits.hpp"

namespace anole
{
/**
 * by allocator,
 *  we can allocate memories for variables/objects/scopes/contexts
*/
template<typename T>
class Allocator
{
  public:
    static_assert(!light::is_void_v<T>);

    using Value = T;
    using Pointer = T *;

  public:
    static Allocator &allocator();

    template<typename U = Value, typename ...Ts>
    static U *alloc(Ts &&...values)
    {
        Allocator::allocator().allocate<U>(std::forward<Ts>(values)...);
    }

    static void dealloc(Pointer ptr)
    {
        Allocator::allocator().deallocate(ptr);
    }

    static void dealloc(void *ptr)
    {
        dealloc(reinterpret_cast<Pointer>(ptr));
    }

    static const std::set<Pointer> &alloced()
    {
        return Allocator::allocator().allocated();
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
        allocated_.insert(ptr);

        Collector::collector().record(Pointer(ptr));

        return ptr;
    }

    void deallocate(Pointer ptr)
    {
        assert(allocated_.count(ptr));
        delete ptr;
        allocated_.erase(ptr);
    }

    void deallocate(void *ptr)
    {
        deallocate(reinterpret_cast<Pointer>(ptr));
    }

    const std::set<Pointer> &allocated()
    {
        return allocated_;
    }

  private:
    std::set<Pointer> allocated_;
};

template<typename T>
inline Allocator<T> &Allocator<T>::allocator()
{
    static Allocator<T> alctor;
    return alctor;
}
} // namespace anole
