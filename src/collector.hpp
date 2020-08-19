#pragma once

#include <set>

namespace anole
{
template<typename T>
class EnableCollect
{
  public:
    EnableCollect(T *bind)
      : bind_(bind)
    {
        // ...
    }

    T *binded()
    {
        return bind_;
    }

  private:
    T *bind_;
};

/**
 * Collector will find variables/objects/scopes/contexts/stacks
 *  which are referenced and then deallocate others
*/
class Collector
{
  public:
    static Collector &collector();

  public:
    Collector() = default;

    /**
     * record allocated address
     *  and only recorded address could be deallocated
     *
     * void record(address);
    */
    template<typename T>
    void record(T *ptr)
    {
        recorded<T>().insert(ptr);
    }

    // TODO
    void gc()
    {
        // ...
    }

  private:
    template<typename T>
    std::set<T *> &recorded()
    {
        static std::set<T *> rcod;
        return rcod;
    }
};

inline Collector &Collector::collector()
{
    static Collector clctor;
    return clctor;
}
} // namespace anole
