#pragma once

namespace ice_language
{
class Object
{
  public:
    virtual ~Object() = 0;
    virtual bool to_bool();
};
}
