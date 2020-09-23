#pragma once

#include "../base.hpp"

namespace anole
{
/**
 * use enum (without class/struct) with namespace
 *  and using token_type::TokenType in order to
 *  make the enum must be accessed using scope
 *  resolution operator and then we can enable
 *  static cast from the enum to its underlying
 *  type
*/
namespace token_type
{
enum TokenType : int
{
    At,        // @

    Use,       // use
    From,      // from
    Prefixop,  // prefixop
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

    Enum,      // enum
    Dict,      // dict

    None,      // none
    True,      // true
    False,     // false

    Identifier, // [^ 0-9_#@.:;?\(\)\[\]\{\}"]+
    Integer,    // [0-9]+
    Double,     // [0-9]+\.[0-9]*
    String,     // "[^"\n]"

    Comma,     // ,
    Dot,       // .
    Dooot,     // ...

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
    TokenType type;
    String value;

    static TokenType add_token_type(const String &str);

    Token() noexcept = default;

    explicit Token(TokenType type, String value = "") noexcept;
    explicit Token(String value) noexcept;

    Token(Token &&other) noexcept;
    Token(const Token &other) noexcept;

    Token &operator=(Token &&other) noexcept;
    Token &operator=(const Token &other) noexcept;
};
}
