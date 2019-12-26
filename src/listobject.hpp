#pragma once

#include <vector>
#include "object.hpp"

namespace ice_language
{
class ListObject : public Object
{
  public:
    ListObject();

    bool to_bool() override;
    std::string to_str() override;
    Ptr<ObjectPtr> index(ObjectPtr) override;
    Ptr<ObjectPtr> load_member(const std::string &name) override;

    std::vector<Ptr<ObjectPtr>> &objects();
    void append(ObjectPtr obj);

  private:
    std::vector<Ptr<ObjectPtr>> objects_;
};
}
