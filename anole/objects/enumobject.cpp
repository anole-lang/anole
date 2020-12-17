#include "objects.hpp"

#include "../runtime/scope.hpp"

using namespace std;

namespace anole
{
EnumObject::EnumObject(SPtr<Scope> scope)
  : Object(ObjectType::Enum)
  , scope_(std::move(scope))
{
    // ...
}

SPtr<Scope> &EnumObject::scope()
{
    return scope_;
}

Address EnumObject::load_member(const String &name)
{
    if (scope_->symbols().count(name))
    {
        return make_shared<Variable>(scope_->load_symbol(name)->ptr());
    }
    return Object::load_member(name);
}

void EnumObject::collect(function<void(Scope *)> func)
{
    func(scope_.get());
}
}
