#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include "IceObject.h"

namespace Ice
{

class Env : public std::enable_shared_from_this<Env>
{
  private:
    std::map<std::string, std::shared_ptr<IceObject>> objects;

  protected:
    std::shared_ptr<Env> prev;

  public:
    Env(std::shared_ptr<Env> prev) : prev(prev) {}
    void put(std::string &, std::shared_ptr<IceObject>);
    std::shared_ptr<IceObject> getObject(std::string &);
};
}

#endif // __CODERUN_H__