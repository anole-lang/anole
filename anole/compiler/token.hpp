#ifndef __ANOLE_COMPILER_TOKEN_HPP__
#define __ANOLE_COMPILER_TOKEN_HPP__

#include "../base.hpp"

namespace anole
{
enum class TokenType : Size
{
    At,         // @

    Use,        // use
    From,       // from
    Prefixop,   // prefixop
    Infixop,    // infixop
    If,         // if
    Elif,       // elif
    Else,       // else
    While,      // while
    Do,         // do
    Foreach,    // foreach
    As,         // as
    Break,      // break
    Continue,   // continue
    Return,     // return
    Match,      // match
    Delay,      // delay

    Enum,       // enum
    Dict,       // dict
    Class,      // class

    None,       // none
    True,       // true
    False,      // false

    Identifier, // [^ 0-9_#@.:;?\(\)\[\]\{\}"]+
    Integer,    // [0-9]+
    Double,     // [0-9]+\.[0-9]*
    String,     // "[^"\n]"

    Comma,      // ,
    Dot,        // .
    Dooot,      // ...

    LParen,     // (
    RParen,     // )
    LBracket,   // [
    RBracket,   // ]
    LBrace,     // {
    RBrace,     // }

    Colon,      // :
    Semicolon,  // ;

    Add,        // +
    Sub,        // -
    Mul,        // *
    Div,        // /
    Mod,        // %

    BAnd,       // &
    BOr,        // |
    BXor,       // ^
    BNeg,       // ~
    BLS,        // <<
    BRS,        // >>

    And,        // and
    Or,         // or
    Not,        // not !

    Is,         // is
    CEQ,        // =
    CNE,        // !=
    CLT,        // <
    CLE,        // <=
    CGT,        // >
    CGE,        // >=

    Ret,        // =>

    Ques,       // ?

    End,
};

/**
 * fst for line_num, snd for char_at_line
*/
using Location = std::pair<Size, Size>;

struct Token
{
    TokenType type;
    String    value;
    Location  location;

  public:
    static TokenType add_token_type(const String &str);

  public:
    Token(TokenType type, Location location) noexcept;
    Token(TokenType type, String value, Location location) noexcept;
    Token(String value, Location location) noexcept;

    Token(Token &&other) noexcept;
    Token(const Token &other) noexcept;

    Token &operator=(Token &&other) noexcept;
    Token &operator=(const Token &other) noexcept;
};
}

#endif
