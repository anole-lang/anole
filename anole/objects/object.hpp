#ifndef __ANOLE_OBJECTS_OBJECT_HPP__
#define __ANOLE_OBJECTS_OBJECT_HPP__

#include "../base.hpp"
#include "../error.hpp"

#include <memory>
#include <functional>

namespace anole
{
class Code;
class Scope;
class Context;
class Variable;
using Address = SPtr<Variable>;

enum class ObjectType : Size
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
    Continuation,
    AnoleModule,
    CppModule,
    Class,
    Method,
    Instance,
};

class Object
{
  public:
    static ObjectType add_object_type(const String &literal);

  public:
    constexpr Object(ObjectType type) noexcept : type_(type) {}
    virtual ~Object() = 0;

    template<ObjectType type>
    bool is() noexcept { return type_ == type; }
    bool is(ObjectType type) noexcept { return type_ == type; }
    Object *type();

  public:
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
