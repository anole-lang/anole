#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

class Token
{
public:
    enum class TOKEN
    {
        TAT, // @

        TIF, // if
        TELSE, // else
        TWHILE, // while
        TRETURN, // return

        TIDENTIFIER, // [a-zA-Z_][a-zA-Z0-9_]*
        TINTEGER, // [0-9]+

        TASSIGN, // :
        TCOMMA, // ,

        TLPAREN, // (
        TRPAREN, // )
        TLBRACE, // {
        TRBRACE, // }

        TADD, // +
        TSUB, // -
        TMUL, // *
        TDIV, // /
        TMOD, // %

        TCEQ, // =
        TEXC, // !
        TCLT, // <
        TCGT, // >
    } token_id;
    std::string value;
    Token(TOKEN token_id, std::string value): token_id(token_id), value(value) {};
    Token(TOKEN token_id): token_id(token_id) {};
    Token(std::string value);
};