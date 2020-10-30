#ifndef __LIB_FILE_FILEOBJECT_HPP__
#define __LIB_FILE_FILEOBJECT_HPP__

#include "../../anole/anole.hpp"

#include <fstream>

namespace anole
{
class FileObject : public Object, public std::enable_shared_from_this<FileObject>
{
  public:
    FileObject(const String &, std::int64_t mode);
    Address load_member(const String &name) override;

    std::fstream &file();

  private:
    std::fstream file_;
};
}

#endif
