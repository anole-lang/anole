#pragma once

#include <string>

namespace anole
{
namespace token_type
{
enum TokenType : int
{
    At,        // @
    AtAt,      // @@

    Use,       // use
    From,      // from
    Infixop,   // infixop
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

    Identifier, // [^ 0-9_#@.:;?\(\)\[\]\{\}"]+
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

    End,
};
}
using token_type::TokenType;

struct Token
{
    Token() = default;
    Token(TokenType type, std::string value = "");
    Token(std::string value);
    Token(Token &&token) noexcept;
    Token(const Token &token);
    Token &operator=(const Token &token);

    static TokenType add_token_type(const std::string &str);

    TokenType type;
    std::string value;
};
}
