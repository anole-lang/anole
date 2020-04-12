#pragma once

#include <string>

namespace ice_language
{
enum class TokenType
{
    End,

    At,        // @
    AtAt,      // @@

    Use,       // use
    From,      // from
    If,        // if
    Elif,      // elif
    Else,      // else
    While,     // while
    Do,        // do
    Foreach,   // foreach
    As,        // as
    Break,     // break
    Continue,  // continue
    Return,    // return
    Match,     // match
    Delay,     // delay

    New,       // new
    Enum,      // enum

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
    Semicolon, // ;
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

    Is,        // is
    CEQ,       // =
    CNE,       // !=
    CLT,       // <
    CLE,       // <=
    CGT,       // >
    CGE,       // >=

    Ret,       // =>

    Ques,      // ?
};

struct Token
{
    Token() = default;
    Token(TokenType type, std::string value = "");
    Token(std::string value);
    Token(Token &&token) noexcept;
    Token(const Token &token);
    Token &operator=(const Token &token);

    TokenType type;
    std::string value;
};
}
