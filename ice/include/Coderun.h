#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include "IceObject.h"

class Env
{
private:
    std::map<std::string, IceObject*> objects;

protected:
    Env *prev;

public:
    Env(Env *prev): prev(prev) {}
    void put(std::string&, IceObject*);
    IceObject *getObject(std::string&);
};

#endif // __CODERUN_H__