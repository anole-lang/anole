#include <string>
#include "token.hpp"

using namespace std;

namespace
{
    constexpr size_t
    operator""_hash(const char *str, size_t len)
    {
        if (len == 0)
        {
            return 0;
        }
        else
        {
            return *str + operator""_hash(str + 1, len - 1) * 29;
        }
    }
}

namespace ice_language
{
Token &Token::operator=(const Token &token)
{
    token_id_ = token.token_id_;
    value_ = token.value_;
    return *this;
}

TokenId Token::token_id() const
{
    return token_id_;
}

void Token::set_token_id(TokenId token_id)
{
    token_id_ = token_id;
}

const string &Token::value() const
{
    return value_;
}

void Token::set_value(std::string value)
{
    value_ = move(value);
}

Token::Token(std::string value) : value_(value)
{
    switch (operator""_hash(value.c_str(), value.size() + 1))
    {
    case "using"_hash:      token_id_ = TokenId::Using;    break;
    case "if"_hash:         token_id_ = TokenId::If;       break;
    case "elif"_hash:       token_id_ = TokenId::Elif;     break;
    case "else"_hash:       token_id_ = TokenId::Else;     break;
    case "while"_hash:      token_id_ = TokenId::While;    break;
    case "do"_hash:         token_id_ = TokenId::Do;       break;
    case "for"_hash:        token_id_ = TokenId::For;      break;
    case "to"_hash:         token_id_ = TokenId::To;       break;
    case "foreach"_hash:    token_id_ = TokenId::Foreach;  break;
    case "as"_hash:         token_id_ = TokenId::As;       break;
    case "break"_hash:      token_id_ = TokenId::Break;    break;
    case "continue"_hash:   token_id_ = TokenId::Continue; break;
    case "return"_hash:     token_id_ = TokenId::Return;   break;
    case "match"_hash:      token_id_ = TokenId::Match;    break;

    case "new"_hash:        token_id_ = TokenId::New;      break;

    case "none"_hash:       token_id_ = TokenId::None;     break;
    case "true"_hash:       token_id_ = TokenId::True;     break;
    case "false"_hash:      token_id_ = TokenId::False;    break;

    case "and"_hash:        token_id_ = TokenId::And;      break;
    case "or"_hash:         token_id_ = TokenId::Or;       break;
    case "not"_hash:        token_id_ = TokenId::Not;      break;

    default:                token_id_ = TokenId::Identifier; break;
    }
}
}
