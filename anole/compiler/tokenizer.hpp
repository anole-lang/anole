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

    std::pair<Size, Size> last_pos();
    String get_err_info(const String &message);

  private:
    void get_next_input();

  private:
    Size cur_line_num_;
    Size last_line_num_;
    Size cur_char_at_line_;
    Size last_char_at_line_;
    String cur_line_;
    String pre_line_;

    std::istream &input_;
    String name_of_input_;
    char last_input_;
};
}

#endif
