#pragma once

#include "base.hpp"
#include "error.hpp"

namespace anole
{
/**
 * use enum (without class/struct) with namespace
 *  and using object_type::ObjectType in order to
 *  make the enum must be accessed using scope
 *  resolution operator and then we can enable
 *  static cast from the enum to its underlying
 *  type
*/
namespace object_type
{
enum ObjectType : int
{
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
    constexpr Object(ObjectType type) noexcept : type_(type) {}
    virtual ~Object() = 0;

    static ObjectType add_object_type(const String &literal);

    template<ObjectType type>
    constexpr bool is() { return type_ == type; }
    ObjectPtr type();

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
