#pragma once

#include "token.hpp"

#include <vector>
#include <istream>
#include <iostream>

namespace anole
{
class Tokenizer
{
  public:
    explicit Tokenizer(std::istream & = std::cin,
        String = "<stdint>");

    Token next();
    void cont();
    void reset();

    std::pair<Size, Size> last_pos()
    {
        return { last_line_num_, last_char_at_line_ };
    }
    String get_err_info(const String &message);

  private:
    void get_next_input();

    Size cur_line_num_;
    Size last_line_num_;
    Size cur_char_at_line_;
    Size last_char_at_line_;
    String cur_line_;
    String pre_line_;

    std::istream &input_stream_;
    String name_of_in_;
    char last_input_;
};
}
