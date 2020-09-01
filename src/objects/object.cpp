#include "objects.hpp"

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace anole
{
namespace
{
vector<String> lc_mapping_t_s
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
map<String, ObjectType> lc_mapping_s_t
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
    { "cont",           ObjectType::Cont            },
    { "anolemodule",    ObjectType::AnoleModule     },
    { "cppmodule",      ObjectType::CppModule       }
};
}

ObjectType Object::add_object_type(const String &literal)
{
    if (!lc_mapping_s_t.count(literal))
    {
        lc_mapping_s_t[literal] = ObjectType(lc_mapping_t_s.size());
        lc_mapping_t_s.push_back(literal);
    }
    return lc_mapping_s_t[literal];
}

Object::~Object() = default;

ObjectSPtr Object::type()
{
    return make_shared<StringObject>(
        lc_mapping_t_s[type_]
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
    return 'p' + to_string(reinterpret_cast<uintptr_t>(this));
}

ObjectSPtr Object::neg()
{
    throw RuntimeError("no neg method");
}

ObjectSPtr Object::add(ObjectRawPtr)
{
    throw RuntimeError("no add method");
}

ObjectSPtr Object::sub(ObjectRawPtr)
{
    throw RuntimeError("no sub method");
}

ObjectSPtr Object::mul(ObjectRawPtr)
{
    throw RuntimeError("no mul method");
}

ObjectSPtr Object::div(ObjectRawPtr)
{
    throw RuntimeError("no div method");
}

ObjectSPtr Object::mod(ObjectRawPtr)
{
    throw RuntimeError("no mod method");
}

/**
 * default = will compare the given objects' address
*/
ObjectSPtr Object::ceq(ObjectRawPtr rptr)
{
    return this == rptr ? BoolObject::the_true() : BoolObject::the_false();
}

ObjectSPtr Object::cne(ObjectRawPtr rptr)
{
    return this != rptr ? BoolObject::the_true() : BoolObject::the_false();
}

ObjectSPtr Object::clt(ObjectRawPtr)
{
    throw RuntimeError("no clt method");
}

ObjectSPtr Object::cle(ObjectRawPtr)
{
    throw RuntimeError("no cle method");
}

ObjectSPtr Object::bneg()
{
    throw RuntimeError("no bneg method");
}

ObjectSPtr Object::bor(ObjectRawPtr)
{
    throw RuntimeError("no bor method");
}

ObjectSPtr Object::bxor(ObjectRawPtr)
{
    throw RuntimeError("no bxor method");
}

ObjectSPtr Object::band(ObjectRawPtr)
{
    throw RuntimeError("no band method");
}

ObjectSPtr Object::bls(ObjectRawPtr)
{
    throw RuntimeError("no bls method");
}

ObjectSPtr Object::brs(ObjectRawPtr)
{
    throw RuntimeError("no brs method");
}

Address Object::index(ObjectSPtr)
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

void Object::collect(function<void(Scope *)>)
{
    // ...
}

void Object::collect(function<void(Context *)>)
{
    // ...
}

void Object::collect(function<void(Variable *)>)
{
    // ...
}
}
