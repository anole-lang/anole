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
    case "using"_hash:      token_id_ = TokenId::TUSING;    break;
    case "if"_hash:         token_id_ = TokenId::TIF;       break;
    case "elif"_hash:       token_id_ = TokenId::TELIF;     break;
    case "else"_hash:       token_id_ = TokenId::TELSE;     break;
    case "while"_hash:      token_id_ = TokenId::TWHILE;    break;
    case "do"_hash:         token_id_ = TokenId::TDO;       break;
    case "for"_hash:        token_id_ = TokenId::TFOR;      break;
    case "to"_hash:         token_id_ = TokenId::TTO;       break;
    case "foreach"_hash:    token_id_ = TokenId::TFOREACH;  break;
    case "as"_hash:         token_id_ = TokenId::TAS;       break;
    case "break"_hash:      token_id_ = TokenId::TBREAK;    break;
    case "continue"_hash:   token_id_ = TokenId::TCONTINUE; break;
    case "return"_hash:     token_id_ = TokenId::TRETURN;   break;
    case "match"_hash:      token_id_ = TokenId::TMATCH;    break;

    case "new"_hash:        token_id_ = TokenId::TNEW;      break;

    case "none"_hash:       token_id_ = TokenId::TNONE;     break;
    case "true"_hash:       token_id_ = TokenId::TTRUE;     break;
    case "false"_hash:      token_id_ = TokenId::TFALSE;    break;

    case "and"_hash:        token_id_ = TokenId::TAND;      break;
    case "or"_hash:         token_id_ = TokenId::TOR;       break;
    case "not"_hash:        token_id_ = TokenId::TNOT;      break;

    default:                token_id_ = TokenId::TIDENTIFIER; break;
    }
}
}
