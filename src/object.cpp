#include "error.hpp"
#include "object.hpp"
#include "boolobject.hpp"
#include "stringobject.hpp"

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
    "module",
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
    { "module",         ObjectType::Module          }
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

ObjectPtr Object::type()
{
    return make_shared<StringObject>(lc_mapping_t_s[type_]);
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

ObjectPtr Object::neg()
{
    throw RuntimeError("no neg method");
}

ObjectPtr Object::add(ObjectPtr)
{
    throw RuntimeError("no add method");
}

ObjectPtr Object::sub(ObjectPtr)
{
    throw RuntimeError("no sub method");
}

ObjectPtr Object::mul(ObjectPtr)
{
    throw RuntimeError("no mul method");
}

ObjectPtr Object::div(ObjectPtr)
{
    throw RuntimeError("no div method");
}

ObjectPtr Object::mod(ObjectPtr)
{
    throw RuntimeError("no mod method");
}

/**
 * default = will compare the given objects' address
*/
ObjectPtr Object::ceq(ObjectPtr ptr)
{
    return this == ptr.get() ? theTrue : theFalse;
}

ObjectPtr Object::cne(ObjectPtr ptr)
{
    return this != ptr.get() ? theTrue : theFalse;
}

ObjectPtr Object::clt(ObjectPtr)
{
    throw RuntimeError("no clt method");
}

ObjectPtr Object::cle(ObjectPtr)
{
    throw RuntimeError("no cle method");
}

ObjectPtr Object::bneg()
{
    throw RuntimeError("no bneg method");
}

ObjectPtr Object::bor(ObjectPtr)
{
    throw RuntimeError("no bor method");
}

ObjectPtr Object::bxor(ObjectPtr)
{
    throw RuntimeError("no bxor method");
}

ObjectPtr Object::band(ObjectPtr)
{
    throw RuntimeError("no band method");
}

ObjectPtr Object::bls(ObjectPtr)
{
    throw RuntimeError("no bls method");
}

ObjectPtr Object::brs(ObjectPtr)
{
    throw RuntimeError("no brs method");
}

Address Object::index(ObjectPtr)
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
}
