#pragma once

#include "object.hpp"
#include "context.hpp"
#include "allocator.hpp"

namespace anole
{
class ContObject : public Object
{
  public:
    friend class Collector;

    ContObject(Context *resume)
      : Object(ObjectType::Cont)
      , resume_(Allocator<Context>::alloc(*resume))
    {
        // ...
    }

    void call(Size num) override;

    Context *resume()
    {
        return resume_;
    }

  private:
    Context *resume_;
};
}
