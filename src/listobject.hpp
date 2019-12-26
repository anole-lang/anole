#pragma once

#include <vector>
#include "scope.hpp"
#include "instanceobject.hpp"

namespace ice_language
{
class ListObject : public Object
{
  public:
    ListObject(std::vector<ObjectPtr> objects);

    bool to_bool() override;
    std::string to_str() override;
    ObjectPtr index(ObjectPtr) override;

  private:
    std::vector<ObjectPtr> objects_;
};
}
