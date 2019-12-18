#pragma once

#include <memory>
#include "object.hpp"

namespace ice_language
{
class NoneObject : public Object
{

};

inline auto TheNoneObject = std::make_shared<NoneObject>();
}
