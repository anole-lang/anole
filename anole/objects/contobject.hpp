#ifndef __ANOLE_OBJECTS_CONT_HPP__
#define __ANOLE_OBJECTS_CONT_HPP__

#include "object.hpp"

namespace anole
{
class ContObject : public Object
{
  public:
    ContObject(SPtr<Context> resume);

    SPtr<Context> resume();

  public:
    void call(Size num) override;

    void collect(std::function<void(Context *)>) override;

  private:
    SPtr<Context> resume_;
};
}

#endif
