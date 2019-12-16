#pragma once

#include <string>
#include "helper.hpp"

namespace ice_language
{
using ObjectPtr = Ptr<class Object>;

class Object
{
  public:
    virtual ~Object() = 0;
    virtual bool to_bool();
    virtual std::string to_str();
    virtual ObjectPtr neg();
    virtual ObjectPtr add(ObjectPtr);
    virtual ObjectPtr sub(ObjectPtr);
    virtual ObjectPtr mul(ObjectPtr);
};
}
