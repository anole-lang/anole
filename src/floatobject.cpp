#include "floatobject.hpp"

using namespace std;

namespace ice_language
{
bool FloatObject::to_bool()
{
    return value_;
}
}
