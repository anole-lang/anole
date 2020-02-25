#pragma once

#include "context.hpp"
#include "object.hpp"

namespace ice_language
{
class ContObject : public Object
{
  public:
    ContObject(Ptr<Context> resume_to)
      : resume_to_(std::make_shared<Context>(*resume_to)) {}

    ObjectPtr ceq(ObjectPtr) override;
    ObjectPtr cne(ObjectPtr) override;

    Ptr<Context> resume_to()
    {
        return resume_to_;
    }

  private:
    Ptr<Context> resume_to_;
};
}
