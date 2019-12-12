#pragma once

#include "code.hpp"
#include "frame.hpp"
#include "parser.hpp"

namespace ice_language
{
class ReadEvalPrintLoop
{
  public:
    ReadEvalPrintLoop() = default;
    void run();
};
}
