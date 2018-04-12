#include "IceObject.h"

IceIntegerObject::IceIntegerObject(long value): value(value)
{
    type = "int";
}

void IceIntegerObject::show()
{
    std::cout << value << std::endl;
}