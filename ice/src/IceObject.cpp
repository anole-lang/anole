#include "IceObject.h"

IceIntegerObject::IceIntegerObject(long value) : value(value)
{
    type = TYPE::INT;
}

void IceIntegerObject::show()
{
    std::cout << value << std::endl;
}

IceObject *IceIntegerObject::add(IceObject *obj)
{
    switch (obj->type)
    {
    case TYPE::INT:
        return new IceIntegerObject(value + dynamic_cast<IceIntegerObject *>(obj)->value);
    default:
        return nullptr;
    }
}

IceObject *IceIntegerObject::sub(IceObject *obj)
{
    switch (obj->type)
    {
    case TYPE::INT:
        return new IceIntegerObject(value - dynamic_cast<IceIntegerObject *>(obj)->value);
    default:
        return nullptr;
    }
}

IceObject *IceIntegerObject::mul(IceObject *obj)
{
    switch (obj->type)
    {
    case TYPE::INT:
        return new IceIntegerObject(value * dynamic_cast<IceIntegerObject *>(obj)->value);
    default:
        return nullptr;
    }
}

IceObject *IceIntegerObject::div(IceObject *obj)
{
    switch (obj->type)
    {
    case TYPE::INT:
        return new IceIntegerObject(value / dynamic_cast<IceIntegerObject *>(obj)->value);
    default:
        return nullptr;
    }
}

IceObject *IceIntegerObject::mod(IceObject *obj)
{
    switch (obj->type)
    {
    case TYPE::INT:
        return new IceIntegerObject(value % dynamic_cast<IceIntegerObject *>(obj)->value);
    default:
        return nullptr;
    }
}