#include "runtime.hpp"

#include "../objects/objects.hpp"

using namespace std;

namespace anole
{
void Collector::gc()
{
    /**
     * collect variables from Context::current()
    */
    collect(Context::current().get());

    auto temp_marked = marked<Variable>();
    for (auto &addr : temp_marked)
    {
        // if cannot visit variable the variable
        if (!collected_.count(addr))
        {
            marked<Variable>().erase(addr);
            Allocator<Variable>::dealloc(addr);
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
        collect(name_addr.second);
    }
}

void Collector::collect_impl(Context *ctx)
{
    collect(ctx->pre_context_.get());
    collect(ctx->scope_.get());
    // different context may share one same stack
    visited_.insert(ctx->stack_.get());

    for (auto &addr : *ctx->stack_)
    {
        collect(addr);
    }
}

void Collector::collect_impl(Variable *addr)
{
    collected_.insert(addr);

    auto rptr = addr->rptr();
    if (!rptr)
    {
        return;
    }

    rptr->collect([this](Scope *scp) { this->collect(scp); });
    rptr->collect([this](Context *ctx) { this->collect(ctx); });
    rptr->collect([this](Variable *var) { this->collect(var); });
}
}
