#pragma once

#include <string>
#include "error.hpp"
#include "helper.hpp"

namespace ice_language
{
using ObjectPtr = Ptr<class Object>;

class Object
{
  public:
    virtual ~Object() = 0;
    virtual bool to_bool();
    virtual std::string to_str();
    virtual std::string to_key();
    virtual ObjectPtr neg();
    virtual ObjectPtr add(ObjectPtr);
    virtual ObjectPtr sub(ObjectPtr);
    virtual ObjectPtr mul(ObjectPtr);
    virtual ObjectPtr div(ObjectPtr);
    virtual ObjectPtr mod(ObjectPtr);
    virtual ObjectPtr ceq(ObjectPtr);
    virtual ObjectPtr cne(ObjectPtr);
    virtual ObjectPtr clt(ObjectPtr);
    virtual ObjectPtr cle(ObjectPtr);
    virtual ObjectPtr bneg();
    virtual ObjectPtr bor(ObjectPtr);
    virtual ObjectPtr bxor(ObjectPtr);
    virtual ObjectPtr band(ObjectPtr);
    virtual ObjectPtr bls(ObjectPtr);
    virtual ObjectPtr brs(ObjectPtr);
    virtual Ptr<ObjectPtr> index(ObjectPtr);
    virtual Ptr<ObjectPtr> load_member(const std::string &name);
};
}
