#include <exception>
#include <stdexcept>
#include "object.hpp"

using namespace std;

namespace ice_language
{
Object::~Object() = default;
bool Object::to_bool()
{
    throw runtime_error("cannot translate to bool");
}
}
