#ifndef __ANOLE_OBJECTS_OBJECT_HPP__
#define __ANOLE_OBJECTS_OBJECT_HPP__

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
    CppModule,
    Class,
    Method,
    Instance,
};
}
using object_type::ObjectType;
class Object;

class Code;
class Scope;
class Context;
class Variable;
using Address = SPtr<Variable>;

class Object
{
  public:
    constexpr Object(ObjectType type) noexcept : type_(type) {}
    virtual ~Object() = 0;

    static ObjectType add_object_type(const String &literal);

    template<ObjectType type>
    constexpr bool is() { return type_ == type; }
    bool is(ObjectType type) { return type_ == type; }
    Object *type();

    virtual bool to_bool();
    virtual String to_str();
    virtual String to_key();

    virtual Object *neg();
    virtual Object *add(Object *);
    virtual Object *sub(Object *);
    virtual Object *mul(Object *);
    virtual Object *div(Object *);
    virtual Object *mod(Object *);
    virtual Object *ceq(Object *);
    virtual Object *cne(Object *);
    virtual Object *clt(Object *);
    virtual Object *cle(Object *);
    virtual Object *bneg();
    virtual Object *bor(Object *);
    virtual Object *bxor(Object *);
    virtual Object *band(Object *);
    virtual Object *bls(Object *);
    virtual Object *brs(Object *);

    virtual Address index(Object *);
    virtual Address load_member(const String &name);

    virtual void call(Size num);
    virtual bool is_callable();

    virtual void collect(std::function<void(Scope *)>);
    virtual void collect(std::function<void(Object *)>);
    virtual void collect(std::function<void(Context *)>);

  private:
    ObjectType type_;
};
}

#endif
