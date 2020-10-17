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

void ClassObject::call(Size num)
{
    auto instance = Allocator<Object>::alloc<InstanceObject>();

    for (auto &sym_addr : scope_->symbols())
    {
        instance->scope()->create_symbol(sym_addr.first, sym_addr.second->ptr());
    }

    /**
     * if scope.count __init__
     *  push instantce
     *  load __init__
     *  call (num + 1)
    */
}
} // namespace anole
