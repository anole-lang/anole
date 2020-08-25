#pragma once

#include "base.hpp"

#include <set>

namespace anole
{
class Scope;
class Context;
class Variable;

/**
 * Collector will find variables
 *  which are referenced and then deallocate others
*/
class Collector
{
  public:
    static Collector &collector()
    {
        static Collector clctor;
        return clctor;
    }

  public:
    /**
     * record allocated address
     *  and only recorded address could be deallocated
     *
     * void record(address);
    */
    template<typename T>
    void mark(T *ptr)
    {
        ++count_;
        if (count_ > 10000)
        {
            count_ = 0;
            gc();
        }

        /**
         * the new allocated variable
         *  must be marked after gc
         *  to ensure it be collected
        */
        marked<T>().insert(ptr);
    }

    template<typename T>
    void collect(T *p)
    {
        if (p == nullptr || visited_.count(p))
        {
            return;
        }
        visited_.insert(p);
        collect_impl(p);
    }

    void gc();

  private:
    /**
     * default ctor is private
     *  in order that we can only use the static collector
    */
    Collector() : count_(0) {}

    template<typename T>
    std::set<T *> &marked()
    {
        /**
         * static mkd for non-static member function
        */
        static std::set<T *> mkd;
        return mkd;
    }

    Size count_;

  private:
    /**
     * collect variables
    */
    std::set<void *> visited_;
    std::set<Variable *> collected_;
    void collect_impl(Scope *);
    void collect_impl(Context *);
    void collect_impl(Variable *);
};
} // namespace anole
