#pragma once

#include "context.hpp"
#include "object.hpp"

namespace ice_language
{
class ContObject : public Object
{
  public:
    ContObject(SPtr<Context> resume)
      : resume_(std::make_shared<Context>(*resume)) {}

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;

    SPtr<Context> resume()
    {
        return resume_;
    }

  private:
    SPtr<Context> resume_;
};
}
