#pragma once

#include "context.hpp"
#include "object.hpp"

namespace anole
{
class ContObject : public Object
{
  public:
    ContObject(SPtr<Context> resume)
      : Object(ObjectType::Cont)
      , resume_(std::make_shared<Context>(*resume)) {}

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;
    void call(std::size_t num) override;
    void call_tail(std::size_t num) override;

    SPtr<Context> resume()
    {
        return resume_;
    }

  private:
    SPtr<Context> resume_;
};
}
