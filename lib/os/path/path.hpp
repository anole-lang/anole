#ifndef __LIB_OS_PATH_HPP__
#define __LIB_OS_PATH_HPP__

#include "../../../anole/anole.hpp"

#include <string>
#include <vector>
#include <filesystem>

class PathObject : public anole::Object
{
  public:
    PathObject(std::filesystem::path path);

    anole::Address load_member(const anole::String &name) override;
    anole::String to_str() override;

    std::filesystem::path &path();

  private:
    std::filesystem::path path_;
};

#endif
