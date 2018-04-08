#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <cstring>

class Env
{
private:
    std::map<std::string, void*> values;
    std::map<std::string, std::string> types;

protected:
    Env *prev;

public:
    Env(Env *prev): prev(prev) {}
    void put(std::string&, void*);
    void *getValue(std::string&);
    std::string getType(std::string&);
};

#endif // __CODERUN_H__