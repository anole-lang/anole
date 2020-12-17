#include "runtime.hpp"

#include "../objects/objects.hpp"

namespace anole
{
void Collector::try_gc()
{
    auto &ref = collector();
    if (ref.count_ > 10000)
    {
        ref.count_ = 0;
        ref.gc();
    }
}

Collector &Collector::collector()
{
    static Collector clctor;
    return clctor;
}

Collector::Collector() noexcept
  : count_(0)
{
    // ...
}

void Collector::gc()
{
    /**
     * collect variables from Context::current()
    */
    collect(Context::current().get());

    auto temp_marked = marked<Object>();
    for (auto ptr : temp_marked)
    {
        // if cannot visit the object
        if (!collected_.count(ptr))
        {
            marked<Object>().erase(ptr);
            Allocator<Object>::dealloc(ptr);
        }
    }

    visited_.clear();
    collected_.clear();
}

void Collector::collect_impl(Scope *scp)
{
    collect(scp->pre_scope_.get());

    for (auto &name_addr : scp->symbols_)
    {
        collect(name_addr.second->ptr());
    }
}

void Collector::collect_impl(Object *ptr)
{
    collected_.insert(ptr);

    ptr->collect([this](Scope *scp) { this->collect(scp); });
    ptr->collect([this](Object *obj) { this->collect(obj); });
    ptr->collect([this](Context *ctx) { this->collect(ctx); });
}

void Collector::collect_impl(Context *ctx)
{
    collect(ctx->pre_context_.get());
    collect(ctx->scope_.get());

    // different context may share one same stack
    if (visited_.count(ctx->stack_.get()))
    {
        return;
    }
    visited_.insert(ctx->stack_.get());

    for (auto &addr : *ctx->stack_)
    {
        collect(addr->ptr());
    }
}
}
