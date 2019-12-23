#pragma once

#include <memory>
#include "object.hpp"

namespace ice_language
{
class NoneObject : public Object
{

};

inline auto theNone = std::make_shared<NoneObject>();
}
