#pragma once

#include <string>
#include <utility>

namespace ice_language
{
struct Token
{
    enum class TOKEN
    {
        TEND,       // #

        TAT,        // @
        TATAT,      // @@

        TUSING,     // using
        TIF,        // if
        TELIF,      // elif
        TELSE,      // else
        TWHILE,     // while
        TDO,        // do
        TFOR,       // for
        TTO,        // to
        TFOREACH,   // foreach
        TAS,        // as
        TBREAK,     // break
        TCONTINUE,  // continue
        TRETURN,    // return
        TMATCH,     // match

        TNEW,       // new

        TNONE,      // none
        TTRUE,      // true
        TFALSE,     // false

        TIDENTIFIER, // [a-zA-Z_][a-zA-Z0-9_]*
        TINTEGER,    // [0-9]+
        TDOUBLE,     // [0-9]+\.[0-9]*
        TSTRING,     // "[^"\n]"

        TASSIGN,    // :
        TCOMMA,     // ,
        TDOT,       // .

        TLPAREN,    // (
        TRPAREN,    // )
        TLBRACKET,  // [
        TRBRACKET,  // ]
        TLBRACE,    // {
        TRBRACE,    // }

        TADD,       // +
        TSUB,       // -
        TMUL,       // *
        TDIV,       // /
        TMOD,       // %

        TBAND,      // &
        TBOR,       // |
        TBXOR,      // ^
        TBNEG,      // ~
        TBLS,       // <<
        TBRS,       // >>

        TAND,       // and
        TOR,        // or
        TNOT,       // not !

        TCEQ,       // =
        TCNE,       // !=
        TCLT,       // <
        TCLE,       // <=
        TCGT,       // >
        TCGE,       // >=

        TRET        // =>
    };

    TOKEN tokenId;
    std::string value;

    Token() = default;
    Token(const Token &token);
    Token(TOKEN tokenId, std::string value)
      : tokenId(tokenId), value(std::move(value)) {}
    Token(TOKEN tokenId) : tokenId(tokenId) {}
    Token(std::string value);
};

using TOKEN = Token::TOKEN;
}
