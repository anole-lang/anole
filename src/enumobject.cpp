#include "enumobject.hpp"

using namespace std;

namespace ice_language
{
SPtr<ObjectPtr> EnumObject::load_member(const std::string &name)
{
    if (scope_->symbols().count(name))
    {
        return make_shared<ObjectPtr>(*scope_->load_symbol(name));
    }
    return Object::load_member(name);
}
}
