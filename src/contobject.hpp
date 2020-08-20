#pragma once

#include "object.hpp"
#include "context.hpp"
#include "allocator.hpp"

namespace anole
{
class ContObject : public Object
{
  public:
    ContObject(SPtr<Context> resume)
      : Object(ObjectType::Cont)
      , resume_(std::make_shared<Context>(*resume))
    {
        // ...
    }

    void call(Size num) override;

    void collect(std::function<void(Context *)>) override;

    SPtr<Context> resume()
    {
        return resume_;
    }

  private:
    SPtr<Context> resume_;
};
}
