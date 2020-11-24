#include "objects.hpp"

#include "../runtime/runtime.hpp"

using namespace std;

namespace anole
{
InstanceObject::InstanceObject()
  : Object(ObjectType::Instance)
  , scope_(make_shared<Scope>(nullptr))
{
    // ...
}

Address InstanceObject::load_member(const String &name)
{
    if (scope_->symbols().count(name))
    {
        auto member = scope_->load_symbol(name);

        if (member->ptr()->is_callable())
        {
            return make_shared<Variable>(
                Allocator<Object>::alloc<MethodObject>(member->ptr(), this)
            );
        }

        return member;
    }
    else
    {
        return scope_->create_symbol(name);
    }
}

void InstanceObject::collect(function<void(Scope *)> func)
{
    func(scope_.get());
}
} // namespace anole
