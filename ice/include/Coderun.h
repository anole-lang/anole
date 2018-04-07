#include <iostream>
#include <map>
#include <string>
#include <cstring>

class BlockExprAST;

class Env
{
private:
    std::map<std::string, void*> values;
    std::map<std::string, std::string> types;

protected:
    Env *prev;

public:
    Env(Env*);
    void put(std::string &name, void* value);
    void *getValue(std::string&);
    std::string getType(std::string&);
};

class Program
{
private:
    Env *top;    
public:
    Program();
};