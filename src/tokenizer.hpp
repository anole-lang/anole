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
    std::string location();

  private:
    std::istream &input_stream_;
    char last_input_;
    u_int32_t cur_line_num_;
    u_int32_t cur_char_at_line_;
    u_int32_t last_line_num_;
    u_int32_t last_char_at_line_;
    std::string cur_line_;
};
}
