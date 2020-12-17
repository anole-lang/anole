#include "objects.hpp"

#include "../runtime/runtime.hpp"
#include "../compiler/compiler.hpp"

using namespace std;

namespace anole
{
ThunkObject::ThunkObject(SPtr<Scope> pre_scope,
    SPtr<Code> code, Size base)
  : Object(ObjectType::Thunk)
  , computed_(false), result_(nullptr)
  , scope_(std::make_shared<Scope>(pre_scope))
  , code_(code), base_(base)
{
    // ...
}

void ThunkObject::set_result(Address res)
{
    result_ = res;
    computed_ = true;
}

bool ThunkObject::computed() const
{
    return computed_;
}

Address ThunkObject::result()
{
    return result_;
}

SPtr<Scope> ThunkObject::scope()
{
    return scope_;
}

SPtr<Code> ThunkObject::code()
{
    return code_;
}

Size ThunkObject::base() const
{
    return base_;
}

void ThunkObject::collect(std::function<void(Scope *)> func)
{
    func(scope_.get());
}

void ThunkObject::collect(std::function<void(Object *)> func)
{
    if (result_)
    {
        func(result_->ptr());
    }
}
}
