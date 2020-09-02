#pragma once

#include "../base.hpp"

#include <set>

namespace anole
{
class Scope;
class Context;
class Variable;

/**
 * Collector will find variables
 *  which are referenced and then deallocate others
 *
 * there is only one global Collector
*/
class Collector
{
  public:
    template<typename T>
    static void mark(T *ptr)
    {
        collector().mark_impl(ptr);
    }

    template<typename T>
    static void collect(T *p)
    {
        auto &ref = collector();
        if (p == nullptr || ref.visited_.count(p))
        {
            return;
        }
        ref.visited_.insert(p);
        ref.collect_impl(p);
    }

  private:
    static Collector &collector()
    {
        static Collector clctor;
        return clctor;
    }

    /**
     * default ctor is private
     *  in order that we can only use the static collector
    */
    Collector() : count_(0) {}

    /**
     * gc can only be called by the collector self
    */
    void gc();

    template<typename T>
    std::set<T *> &marked()
    {
        /**
         * static mkd for non-static member function
        */
        static std::set<T *> mkd;
        return mkd;
    }

    /**
     * record allocated address
     *  and only recorded address could be deallocated
     *
     * void record(address);
    */
    template<typename T>
    void mark_impl(T *ptr)
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

    Size count_;

  private:
    /**
     * collect variables
    */
    void collect_impl(Scope *);
    void collect_impl(Context *);
    void collect_impl(Variable *);
    std::set<void *> visited_;
    std::set<Variable *> collected_;
};
} // namespace anole
