#pragma once

#include "../base.hpp"
#include "../error.hpp"

#include <memory>
#include <functional>

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
    AnoleModule,
    CppModule
};
}
using object_type::ObjectType;

class Object;
using ObjectSPtr = SPtr<Object>;
using ObjectRawPtr = Object *;

class Scope;
class Context;
class Variable;
using Address = Variable *;

class Object
{
  public:
    constexpr Object(ObjectType type) noexcept : type_(type) {}
    virtual ~Object() = 0;

    static ObjectType add_object_type(const String &literal);

    template<ObjectType type>
    constexpr bool is() { return type_ == type; }
    bool is(ObjectType type) { return type_ == type; }
    ObjectSPtr type();

    virtual bool to_bool();
    virtual String to_str();
    virtual String to_key();

    virtual ObjectSPtr neg();
    virtual ObjectSPtr add(ObjectRawPtr);
    virtual ObjectSPtr sub(ObjectRawPtr);
    virtual ObjectSPtr mul(ObjectRawPtr);
    virtual ObjectSPtr div(ObjectRawPtr);
    virtual ObjectSPtr mod(ObjectRawPtr);
    virtual ObjectSPtr ceq(ObjectRawPtr);
    virtual ObjectSPtr cne(ObjectRawPtr);
    virtual ObjectSPtr clt(ObjectRawPtr);
    virtual ObjectSPtr cle(ObjectRawPtr);
    virtual ObjectSPtr bneg();
    virtual ObjectSPtr bor(ObjectRawPtr);
    virtual ObjectSPtr bxor(ObjectRawPtr);
    virtual ObjectSPtr band(ObjectRawPtr);
    virtual ObjectSPtr bls(ObjectRawPtr);
    virtual ObjectSPtr brs(ObjectRawPtr);

    virtual Address index(ObjectSPtr);
    virtual Address load_member(const String &name);

    virtual void call(Size num);

    virtual void collect(std::function<void(Scope *)>);
    virtual void collect(std::function<void(Context *)>);
    virtual void collect(std::function<void(Variable *)>);

  private:
    ObjectType type_;
};
}
