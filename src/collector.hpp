#pragma once

namespace anole
{
/**
 * Collector will find objects/scopes/contexts
 *  which are referenced and then deallocate others
*/
class Collector
{
  public:
    Collector() = default;

    /**
     * record allocated address
     *
     * void record(Address);
    */

    void gc();
};
} // namespace anole
