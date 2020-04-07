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

string Object::to_key()
{
    return "";
}

ObjectPtr Object::neg()
{
    throw runtime_error("no neg method");
}

ObjectPtr Object::add(ObjectPtr)
{
    throw runtime_error("no add method");
}

ObjectPtr Object::sub(ObjectPtr)
{
    throw runtime_error("no sub method");
}

ObjectPtr Object::mul(ObjectPtr)
{
    throw runtime_error("no mul method");
}

ObjectPtr Object::div(ObjectPtr)
{
    throw runtime_error("no div method");
}

ObjectPtr Object::mod(ObjectPtr)
{
    throw runtime_error("no mod method");
}

ObjectPtr Object::ceq(ObjectPtr)
{
    throw runtime_error("no ceq method");
}

ObjectPtr Object::cne(ObjectPtr)
{
    throw runtime_error("no cne method");
}

ObjectPtr Object::clt(ObjectPtr)
{
    throw runtime_error("no clt method");
}

ObjectPtr Object::cle(ObjectPtr)
{
    throw runtime_error("no cle method");
}

ObjectPtr Object::bneg()
{
    throw runtime_error("no bneg method");
}

ObjectPtr Object::bor(ObjectPtr)
{
    throw runtime_error("no bor method");
}

ObjectPtr Object::bxor(ObjectPtr)
{
    throw runtime_error("no bxor method");
}

ObjectPtr Object::band(ObjectPtr)
{
    throw runtime_error("no band method");
}

ObjectPtr Object::bls(ObjectPtr)
{
    throw runtime_error("no bls method");
}

ObjectPtr Object::brs(ObjectPtr)
{
    throw runtime_error("no brs method");
}

Ptr<ObjectPtr> Object::index(ObjectPtr)
{
    throw runtime_error("not support index");
}

Ptr<ObjectPtr> Object::load_member(const string &name)
{
    throw runtime_error("no member named " + name);
}
}
