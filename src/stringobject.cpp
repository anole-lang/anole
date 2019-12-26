#include "stringobject.hpp"

using namespace std;

namespace ice_language
{
bool StringObject::to_bool()
{
    return !value_.empty();
}

string StringObject::to_str()
{
    return value_;
}
}
