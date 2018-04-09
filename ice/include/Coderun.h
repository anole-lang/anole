#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include "Object.h"

class Env
{
private:
    std::map<std::string, Object*> objects;

protected:
    Env *prev;

public:
    Env(Env *prev): prev(prev) {}
    void put(std::string&, Object*);
    Object *getObject(std::string&);
};

#endif // __CODERUN_H__