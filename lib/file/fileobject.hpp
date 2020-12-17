#ifndef __LIB_FILE_FILEOBJECT_HPP__
#define __LIB_FILE_FILEOBJECT_HPP__

#include "../../anole/anole.hpp"

#include <fstream>

class FileObject
  : public anole::Object
{
  public:
    FileObject(const anole::String &, std::int64_t mode);
    anole::Address load_member(const anole::String &name) override;

    std::fstream &file();

  private:
    std::fstream file_;
};

#endif
