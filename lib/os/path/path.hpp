#include "../../../src/object.hpp"

#include <string>
#include <vector>
#include <filesystem>

namespace anole
{
class PathObject : public Object
{
  public:
    PathObject(std::filesystem::path path);

    Address load_member(const String &name) override;
    String to_str() override;

    std::filesystem::path &path();

  private:
    std::filesystem::path path_;
};
}
