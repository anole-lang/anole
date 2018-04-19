#include "IceObject.h"

IceIntegerObject::IceIntegerObject(long value): value(value)
{
    type = TYPE::INT;
}

void IceIntegerObject::show()
{
    std::cout << value << std::endl;
}

IceObject *IceIntegerObject::add(IceObject *obj)
{
    IceObject *res_obj = nullptr;
    switch(obj->type)
    {
        case TYPE::INT:
            res_obj = new IceIntegerObject(value + dynamic_cast<IceIntegerObject *>(obj)->value);
            break;
        default:
            break;
    }
    return res_obj;
}

IceObject *IceIntegerObject::mul(IceObject *obj)
{
    IceObject *res_obj = nullptr;
    switch(obj->type)
    {
        case TYPE::INT:
            res_obj = new IceIntegerObject(value * dynamic_cast<IceIntegerObject *>(obj)->value);
            break;
        default:
            break;
    }
    return res_obj;
}