#pragma once

#include <vector>
#include "object.hpp"

namespace anole
{
class ListObject : public Object
{
  public:
    ListObject();

    bool to_bool() override;
    std::string to_str() override;
    std::string to_key() override;
    SPtr<ObjectPtr> index(ObjectPtr) override;
    SPtr<ObjectPtr> load_member(const std::string &name) override;

    std::vector<SPtr<ObjectPtr>> &objects();
    void append(ObjectPtr obj);

  private:
    std::vector<SPtr<ObjectPtr>> objects_;
};
}
