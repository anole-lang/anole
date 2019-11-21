#pragma once

#include <vector>
#include <string>
#include <istream>
#include <iostream>
#include "token.hpp"

namespace ice_language
{
class Tokenizer
{
  public:
    Tokenizer(std::istream &in = std::cin);
    Token next();

  private:
    std::istream &input_stream_;
    char last_input_;
};
}
