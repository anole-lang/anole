#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

class Token
{
public:
    enum TOKEN
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
        TCNE, // !=
        TCLT, // <
        TCLE, // <=
        TCGT, // >
        TCGE // >=
    } token_id;
    std::string value;
};