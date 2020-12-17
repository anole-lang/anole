#include "objects.hpp"

#include "../runtime/runtime.hpp"

namespace anole
{
InstanceObject::InstanceObject()
  : Object(ObjectType::Instance)
  , scope_(std::make_shared<Scope>(nullptr))
{
    // ...
}

SPtr<Scope> &InstanceObject::scope()
{
    return scope_;
}

Address InstanceObject::load_member(const String &name)
{
    if (scope_->symbols().count(name))
    {
        auto member = scope_->load_symbol(name);

        if (member->ptr()->is_callable())
        {
            return std::make_shared<Variable>(
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

void InstanceObject::collect(std::function<void(Scope *)> func)
{
    func(scope_.get());
}
} // namespace anole
