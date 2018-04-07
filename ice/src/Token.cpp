#include "Token.h"

Token::Token(std::string value): value(value)
{
    if (value == "if")
        token_id = TOKEN::TIF;
    else if (value == "else")
        token_id = TOKEN::TELSE;
    else if (value == "return")
        token_id = TOKEN::TRETURN;
    else
        token_id = TOKEN::TIDENTIFIER;
}