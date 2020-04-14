#include <exception>
#include <stdexcept>
#include "object.hpp"

using namespace std;

namespace anole
{
Object::~Object() = default;

bool Object::to_bool()
{
    throw RuntimeError("cannot translate to bool");
}

string Object::to_str()
{
    return "<no definition of to_str>";
}

string Object::to_key()
{
    return 'p' + to_string(reinterpret_cast<uintptr_t>(this));
}

ObjectPtr Object::neg()
{
    throw RuntimeError("no neg method");
}

ObjectPtr Object::add(ObjectPtr)
{
    throw RuntimeError("no add method");
}

ObjectPtr Object::sub(ObjectPtr)
{
    throw RuntimeError("no sub method");
}

ObjectPtr Object::mul(ObjectPtr)
{
    throw RuntimeError("no mul method");
}

ObjectPtr Object::div(ObjectPtr)
{
    throw RuntimeError("no div method");
}

ObjectPtr Object::mod(ObjectPtr)
{
    throw RuntimeError("no mod method");
}

ObjectPtr Object::ceq(ObjectPtr)
{
    throw RuntimeError("no ceq method");
}

ObjectPtr Object::cne(ObjectPtr)
{
    throw RuntimeError("no cne method");
}

ObjectPtr Object::clt(ObjectPtr)
{
    throw RuntimeError("no clt method");
}

ObjectPtr Object::cle(ObjectPtr)
{
    throw RuntimeError("no cle method");
}

ObjectPtr Object::bneg()
{
    throw RuntimeError("no bneg method");
}

ObjectPtr Object::bor(ObjectPtr)
{
    throw RuntimeError("no bor method");
}

ObjectPtr Object::bxor(ObjectPtr)
{
    throw RuntimeError("no bxor method");
}

ObjectPtr Object::band(ObjectPtr)
{
    throw RuntimeError("no band method");
}

ObjectPtr Object::bls(ObjectPtr)
{
    throw RuntimeError("no bls method");
}

ObjectPtr Object::brs(ObjectPtr)
{
    throw RuntimeError("no brs method");
}

SPtr<ObjectPtr> Object::index(ObjectPtr)
{
    throw RuntimeError("not support index");
}

SPtr<ObjectPtr> Object::load_member(const string &name)
{
    throw RuntimeError("no member named " + name);
}
}
