#include "dictobject.hpp"

using namespace std;

namespace ice_language
{
DictObject::DictObject() = default;

bool DictObject::to_bool()
{
    return !data_.empty();
}

string DictObject::to_str()
{
    string res = "{";
    for (auto it = data_.begin();
        it != data_.end(); ++it)
    {
        if (it != data_.begin())
        {
            res += ",";
        }
        res += " " + it->first->to_str() + " => " + (*it->second)->to_str();
    }
    return res + " }";
}

string DictObject::to_key()
{
    return 'd' + to_str();
}

Ptr<ObjectPtr> DictObject::index(ObjectPtr index)
{
    return data_[index];
}

Ptr<ObjectPtr> DictObject::load_member(const string &name)
{
    // ...
    return nullptr;
}

void DictObject::insert(ObjectPtr key, ObjectPtr value)
{
    data_[key] = make_shared<ObjectPtr>(value);
}
}
