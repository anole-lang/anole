#include "Object.h"

void Object::show()
{
    if (type == "int")
    {
        std::cout << *(long long *)value << std::endl;
    }
}