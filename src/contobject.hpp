#pragma once

#include "frame.hpp"
#include "object.hpp"

namespace ice_language
{
class ContObject : public Object
{
  public:
    ContObject(Ptr<Frame> resume_to)
      : resume_to_(resume_to) {}

    Ptr<Frame> resume_to()
    {
        return resume_to_;
    }

  private:
    Ptr<Frame> resume_to_;
};
}
