#include "objects.hpp"

#include "../runtime/runtime.hpp"

using namespace std;

namespace anole
{
ClassObject::ClassObject(String name,
    SPtr<Scope> pre_scope)
  : Object(ObjectType::Class)
  , name_(move(name))
  , scope_(make_shared<Scope>(pre_scope))
{
    // ...
}

Address ClassObject::load_member(const String &name)
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
        return Object::load_member(name);
    }
}

/**
 * NOTE: not consider inherited constructors now
*/
void ClassObject::call(Size num)
{
    auto instance = Allocator<Object>::alloc<InstanceObject>();

    for (auto &sym_addr : scope_->symbols())
    {
        instance->scope()->create_symbol(sym_addr.first, sym_addr.second->ptr());
    }

    if (scope_->symbols().count("__init__"))
    {
        /**
         * __init__ is a special method to construct object
         *  and it doesn't have return value
         *
         * this is be done in the parse phase
        */
        Context::current()->push(instance);
        auto ctor = scope_->load_symbol("__init__");
        ctor->ptr()->call(num + 1);
    }
    else if (num == 0)
    {
        ++Context::current()->pc();
    }
    else
    {
        throw RuntimeError("only default ctor but given non-zero arguments");
    }

    Context::current()->push(instance);
}

void ClassObject::collect(std::function<void(Scope *)> func)
{
    func(scope_.get());
}
} // namespace anole
