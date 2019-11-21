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
        End,       // #

        At,        // @
        AtAt,      // @@

        Using,     // using
        If,        // if
        Elif,      // elif
        Else,      // else
        While,     // while
        Do,        // do
        For,       // for
        To,        // to
        Foreach,   // foreach
        As,        // as
        Break,     // break
        Continue,  // continue
        Return,    // return
        Match,     // match

        New,       // new

        None,      // none
        True,      // true
        False,     // false

        Identifier, // [a-zA-Z_][a-zA-Z0-9_]*
        Integer,    // [0-9]+
        Double,     // [0-9]+\.[0-9]*
        String,     // "[^"\n]"

        Assign,    // :
        Comma,     // ,
        Dot,       // .

        LParen,    // (
        RParen,    // )
        LBracket,  // [
        RBracket,  // ]
        LBrace,    // {
        RBrace,    // }

        Add,       // +
        Sub,       // -
        Mul,       // *
        Div,       // /
        Mod,       // %

        BAnd,      // &
        BOr,       // |
        BXor,      // ^
        BNeg,      // ~
        BLS,       // <<
        BRS,       // >>

        And,       // and
        Or,        // or
        Not,       // not !

        CEQ,       // =
        CNE,       // !=
        CLT,       // <
        CLE,       // <=
        CGT,       // >
        CGE,       // >=

        Ret        // =>
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
