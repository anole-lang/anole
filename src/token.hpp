#pragma once

#include <string>
#include <utility>

namespace ice_language
{
class Token
{
  public:
    enum class TokenId
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

    Token() = default;
    Token(TokenId token_id, std::string value)
      : token_id_(token_id), value_(std::move(value)) {}
    Token(TokenId token_id) : token_id_(token_id) {}
    Token(std::string value);
    Token(Token &&token)
      : token_id_(token.token_id_),
        value_(std::move(token.value_)) {}
    Token(const Token &token)
      : token_id_(token.token_id_),
        value_(token.value_) {}
    Token &operator=(const Token &token);

    TokenId token_id() const;
    void set_token_id(TokenId token_id);

    const std::string &value() const;
    void set_value(std::string value);

  private:
    TokenId token_id_;
    std::string value_;
};
using TokenId = Token::TokenId;
}
