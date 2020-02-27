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
    Tokenizer(std::istream & = std::cin, std::string = "<stdint>");
    Token next();
    void cont();
    void reset();
    std::string get_err_info(const std::string &message);

  private:
    void get_next_input();

    std::size_t cur_line_num_;
    std::size_t last_line_num_;
    std::size_t cur_char_at_line_;
    std::size_t last_char_at_line_;
    std::string cur_line_;
    std::string pre_line_;

    std::istream &input_stream_;
    std::string name_of_in_;
    char last_input_;
};
}
