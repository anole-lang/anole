#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <iostream>
#include <string>
#include <cstring>

class Object
{
protected:
    void *value;
    std::string type;

public:
    Object(void *value, std::string type): value(value), type(type) {}
    void show();
};

#endif // __OBJECT_H__