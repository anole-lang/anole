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

string Object::to_str()
{
    return "";
}

ObjectPtr Object::neg()
{
    throw runtime_error("no neg method");
}

ObjectPtr Object::add(ObjectPtr obj)
{
    throw runtime_error("no add method");
}

ObjectPtr Object::sub(ObjectPtr obj)
{
    throw runtime_error("no sub method");
}

ObjectPtr Object::mul(ObjectPtr obj)
{
    throw runtime_error("no mul method");
}

ObjectPtr Object::div(ObjectPtr obj)
{
    throw runtime_error("no div method");
}

ObjectPtr Object::mod(ObjectPtr obj)
{
    throw runtime_error("no mod method");
}

ObjectPtr Object::ceq(ObjectPtr obj)
{
    throw runtime_error("no ceq method");
}

ObjectPtr Object::cne(ObjectPtr obj)
{
    throw runtime_error("no cne method");
}

ObjectPtr Object::clt(ObjectPtr obj)
{
    throw runtime_error("no clt method");
}

ObjectPtr Object::cle(ObjectPtr obj)
{
    throw runtime_error("no cle method");
}
}
