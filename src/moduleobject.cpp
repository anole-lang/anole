#include "moduleobject.hpp"

using namespace std;

namespace ice_language
{
Ptr<ObjectPtr> ModuleObject::load_member(const string &name)
{
    if (scope_->symbols().count(name))
    {
        return scope_->load_symbol(name);
    }
    return Object::load_member(name);
}
}
