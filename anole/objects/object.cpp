#include "objects.hpp"

#include "../runtime/allocator.hpp"

#include <map>
#include <vector>
#include <algorithm>

namespace anole
{
namespace // local
{
std::vector<String> localMappingTypeStr
{
    "none",
    "boolean",
    "integer",
    "float",
    "string",
    "enum",
    "list",
    "listiterator",
    "dict",
    "builtinfunc",
    "func",
    "thunk",
    "cont",
    "anolemodule",
    "cppmodule"
};
std::map<String, ObjectType> localMappingStrType
{
    { "none",           ObjectType::None            },
    { "boolean",        ObjectType::Boolean         },
    { "integer",        ObjectType::Integer         },
    { "float",          ObjectType::Float           },
    { "string",         ObjectType::String          },
    { "enum",           ObjectType::Enum            },
    { "list",           ObjectType::List            },
    { "listiterator",   ObjectType::ListIterator    },
    { "dict",           ObjectType::Dict            },
    { "builtinfunc",    ObjectType::BuiltinFunc     },
    { "func",           ObjectType::Func            },
    { "thunk",          ObjectType::Thunk           },
    { "continuation",   ObjectType::Continuation    },
    { "anolemodule",    ObjectType::AnoleModule     },
    { "cppmodule",      ObjectType::CppModule       }
};
}

ObjectType Object::add_object_type(const String &literal)
{
    auto find = localMappingStrType.find(literal);
    if (find == localMappingStrType.end())
    {
        localMappingTypeStr.push_back(literal);
        return localMappingStrType[literal] = static_cast<ObjectType>(localMappingTypeStr.size());
    }
    return find->second;
}

Object::~Object() = default;

Object *Object::type()
{
    return Allocator<Object>::alloc<StringObject>(
        localMappingTypeStr[static_cast<Size>(type_)]
    );
}

bool Object::to_bool()
{
    throw RuntimeError("cannot translate to bool");
}

String Object::to_str()
{
    return "<no definition of to_str>";
}

String Object::to_key()
{
    return 'p' + std::to_string(reinterpret_cast<uintptr_t>(this));
}

Object *Object::neg()
{
    throw RuntimeError("no neg method");
}

Object *Object::add(Object *)
{
    throw RuntimeError("no add method");
}

Object *Object::sub(Object *)
{
    throw RuntimeError("no sub method");
}

Object *Object::mul(Object *)
{
    throw RuntimeError("no mul method");
}

Object *Object::div(Object *)
{
    throw RuntimeError("no div method");
}

Object *Object::mod(Object *)
{
    throw RuntimeError("no mod method");
}

/**
 * default = will compare the given objects' address
*/
Object *Object::ceq(Object *rptr)
{
    return this == rptr ? BoolObject::the_true() : BoolObject::the_false();
}

Object *Object::cne(Object *rptr)
{
    return this != rptr ? BoolObject::the_true() : BoolObject::the_false();
}

Object *Object::clt(Object *)
{
    throw RuntimeError("no clt method");
}

Object *Object::cle(Object *)
{
    throw RuntimeError("no cle method");
}

Object *Object::bneg()
{
    throw RuntimeError("no bneg method");
}

Object *Object::bor(Object *)
{
    throw RuntimeError("no bor method");
}

Object *Object::bxor(Object *)
{
    throw RuntimeError("no bxor method");
}

Object *Object::band(Object *)
{
    throw RuntimeError("no band method");
}

Object *Object::bls(Object *)
{
    throw RuntimeError("no bls method");
}

Object *Object::brs(Object *)
{
    throw RuntimeError("no brs method");
}

Address Object::index(Object *)
{
    throw RuntimeError("not support index");
}

Address Object::load_member(const String &name)
{
    throw RuntimeError("no member named " + name);
}

void Object::call(Size arg_num)
{
    throw RuntimeError("failed call with the given non-function");
}

bool Object::is_callable()
{
    return false;
}

void Object::collect(std::function<void(Scope *)>)
{
    // ...
}

void Object::collect(std::function<void(Object *)>)
{
    // ...
}

void Object::collect(std::function<void(Context *)>)
{
    // ...
}
}
