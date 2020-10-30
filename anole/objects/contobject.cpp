#include "objects.hpp"

#include "../runtime/runtime.hpp"

using namespace std;

namespace anole
{
ContObject::ContObject(SPtr<Context> resume)
  : Object(ObjectType::Cont)
  , resume_(std::make_shared<Context>(*resume))
{
    // ...
}

void ContObject::call(Size n)
{
    if (n != 1)
    {
        throw RuntimeError("continuation need a argument");
    }
    auto retval = Context::current()->pop_ptr();
    Context::current() = make_shared<Context>(resume_);
    Context::current()->push(retval);
    ++Context::current()->pc();
}

void ContObject::collect(function<void(Context *)> func)
{
    func(resume_.get());
}
}
