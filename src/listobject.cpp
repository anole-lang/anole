#include "boolobject.hpp"
#include "listobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
ListObject::ListObject(std::vector<ObjectPtr> objects)
  : objects_(std::move(objects)) {}

bool ListObject::to_bool()
{
    return !objects_.empty();
}

string ListObject::to_str()
{
    string res = "[";
    for (size_t i = 0; i < objects_.size(); ++i)
    {
        if (i) res += ", ";
        res += objects_[i]->to_str();
    }
    return res + "]";
}

ObjectPtr ListObject::index(ObjectPtr index)
{
    if (auto p = dynamic_pointer_cast<IntegerObject>(index))
    {
        return objects_[p->value()];
    }
    else
    {
        throw runtime_error("index should be integer");
    }
}
}
