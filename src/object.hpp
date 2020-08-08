#pragma once

#include "base.hpp"
#include "error.hpp"

namespace anole
{
namespace object_type
{
enum ObjectType : int
{
    NotDefined,
    None,
    Boolean,
    Integer,
    Float,
    String,
    Enum,
    List,
    ListIterator,
    Dict,
    BuiltinFunc,
    Func,
    Thunk,
    Cont,
    Module,
};
}

using object_type::ObjectType;
using ObjectPtr = SPtr<class Object>;
using Address = SPtr<ObjectPtr>;

class Object
{
  public:
    constexpr Object() noexcept : type_(ObjectType::NotDefined) {}
    constexpr Object(ObjectType type) noexcept : type_(type) {}

    constexpr ObjectType type() const noexcept { return type_; }

    virtual ~Object() = 0;
    virtual bool to_bool();
    virtual String to_str();
    virtual String to_key();
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
    virtual Address index(ObjectPtr);
    virtual Address load_member(const String &name);
    virtual void call(Size num);

  private:
    ObjectType type_;
};
}
