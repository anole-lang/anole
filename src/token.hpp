#pragma once

#include <string>
#include <utility>

namespace ice_language
{
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
    Delay,     // delay

    New,       // new

    None,      // none
    True,      // true
    False,     // false

    Identifier, // [a-zA-Z_][a-zA-Z0-9_]*
    Integer,    // [0-9]+
    Double,     // [0-9]+\.[0-9]*
    String,     // "[^"\n]"

    Comma,     // ,
    Dot,       // .

    LParen,    // (
    RParen,    // )
    LBracket,  // [
    RBracket,  // ]
    LBrace,    // {
    RBrace,    // }

    Colon,     // :
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

struct Token
{
  public:
    Token() = default;
    Token(TokenId token_id, std::string value = "");
    Token(std::string value);
    Token(Token &&token);
    Token(const Token &token);
    Token &operator=(const Token &token);

    TokenId token_id;
    std::string value;
};
}
