#include <string>
#include <fstream>
#include "../../src/object.hpp"

extern "C"
{
void _open();
}

namespace ice_language
{
class FileObject : public Object
{
  public:
    FileObject(const std::string &, std::int64_t mode);
    SPtr<ObjectPtr> load_member(const std::string &name) override;

    std::fstream &file();

  private:
    std::fstream file_;
};
}
