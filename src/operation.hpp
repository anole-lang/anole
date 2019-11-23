#pragma once

#include <tuple>

namespace ice_language
{
enum class Opcode
{
    PUSH
};

class OprandsBase {};

template <typename ...Types>
class Oprands : public OprandsBase
{
  public:
    Oprands(Types ...args) : values_(args...) {}

  private:
    std::tuple<Types> values_;
};
}
