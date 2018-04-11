#include "IceObject.h"

IceIntegerObject::IceIntegerObject(long value)
{
    type = "int";
    this->value = new long(value);
}

void IceIntegerObject::show()
{
    std::cout << *(long *)value << std::endl;
}