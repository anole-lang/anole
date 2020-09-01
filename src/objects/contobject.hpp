#pragma once

#include "object.hpp"

namespace anole
{
class ContObject : public Object
{
  public:
    ContObject(SPtr<Context> resume);

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
