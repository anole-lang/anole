#include <string>
#include <fstream>
#include "../../src/object.hpp"

extern "C"
{
void __open();
}

namespace anole
{
class FileObject : public Object, public std::enable_shared_from_this<FileObject>
{
  public:
    FileObject(const std::string &, std::int64_t mode);
    Address load_member(const std::string &name) override;

    std::fstream &file();

  private:
    std::fstream file_;
};
}
