#ifndef __ANOLE_COMPILER_TOKENIZER_HPP__
#define __ANOLE_COMPILER_TOKENIZER_HPP__

#include "token.hpp"

#include <vector>
#include <istream>
#include <iostream>

namespace anole
{
class Tokenizer
{
  public:
    Tokenizer(std::istream &input, String name_of_input) noexcept;

    Token next_token();
    void resume();
    void reset();

    String get_err_info(const String &message);

  private:
    void get_next_input();

  private:
    Location cur_location_;
    Location last_location_;
    String cur_line_;
    String last_line_;

    std::istream &input_;
    String name_of_input_;
    char last_input_;
};
}

#endif
