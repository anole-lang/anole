#pragma once

#include <tuple>

namespace ice_language
{
enum class Opcode
{
    Push,
    Add
};

class OprandsBase {};

template <typename ...Types>
class Oprands : public OprandsBase
{
  public:
    Oprands(Types ...args) : values_(args...) {}
    std::tuple<Types> &get_values()
    {
        return values_;
    }

  private:
    std::tuple<Types> values_;
};
}
