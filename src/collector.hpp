#pragma once

#include "base.hpp"

#include <set>

namespace anole
{
/**
 * Collector will find variables/objects/scopes/contexts/stacks
 *  which are referenced and then deallocate others
*/
class Collector
{
  public:
    static Collector &collector();

  public:
    Collector() : count_(0) {}

    /**
     * record allocated address
     *  and only recorded address could be deallocated
     *
     * void record(address);
    */
    template<typename T>
    void mark(T *ptr)
    {
        marked<T>().insert(ptr);
        ++count_;
        if (count_ > 1000)
        {
            gc();
            count_ = 0;
        }
    }

    void gc();

  private:
    template<typename T>
    std::set<T *> &marked()
    {
        static std::set<T *> mkd;
        return mkd;
    }

    Size count_;
};

inline Collector &Collector::collector()
{
    static Collector clctor;
    return clctor;
}
} // namespace anole
