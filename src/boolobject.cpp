#include "boolobject.hpp"

using namespace std;

namespace anole
{
bool BoolObject::to_bool()
{
    return value_;
}

string BoolObject::to_str()
{
    return value_ ? "true" : "false";
}
}
