#include "allocator.hpp"
#include "contobject.hpp"
#include "boolobject.hpp"

using namespace std;

namespace anole
{
void ContObject::call(Size n)
{
    if (n != 1)
    {
        throw RuntimeError("continuation need a argument");
    }
    auto retval = Context::current()->pop_sptr();
    Context::current() = make_shared<Context>(resume_);
    Context::current()->push(retval);
    ++Context::current()->pc();
}

void ContObject::collect(function<void(Context *)> func)
{
    func(resume_.get());
}
}
