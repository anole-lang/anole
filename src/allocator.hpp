#pragma once

#include <list>

namespace anole
{
/**
 * by allocator,
 *  we can allocate memories for variables
 *  and every variable could reference to one object
 *
 * objects are also allocated by allocator
 *  and then bind variables with objects by var.bind()
*/
class Allocator
{
  public:
    Allocator() = default;

  private:
};
} // namespace anole
