#pragma once

#include <tuple>

namespace ice_language
{
enum class Opcode
{

};

class OprandsBase {};

template <Opcode opcode, typename ...Types>
class Oprands : public OprandBase
{
  public:
    Oprands(Types ...args) : values_(args...) {}

  private:
    std::tuple<Types> values_;
};
}
