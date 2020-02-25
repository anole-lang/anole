#pragma once

#include "context.hpp"
#include "object.hpp"

namespace ice_language
{
class ContObject : public Object
{
  public:
    ContObject(Ptr<Context> resume)
      : resume_(std::make_shared<Context>(*resume)) {}

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;

    Ptr<Context> resume()
    {
        return resume_;
    }

  private:
    Ptr<Context> resume_;
};
}
