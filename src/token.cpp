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
Token::Token(TokenId token_id, std::string value)
    : token_id(token_id), value(std::move(value)) {}

Token::Token(Token &&token) noexcept
    : token_id(token.token_id),
    value(std::move(token.value)) {}

Token::Token(const Token &token)
    : token_id(token.token_id),
    value(token.value) {}

Token &Token::operator=(const Token &token)
{
    token_id = token.token_id;
    value = token.value;
    return *this;
}

Token::Token(std::string value) : value(value)
{
    switch (operator""_hash(value.c_str(), value.size() + 1))
    {
    case "use"_hash:      token_id = TokenId::Use;    break;
    case "if"_hash:         token_id = TokenId::If;       break;
    case "elif"_hash:       token_id = TokenId::Elif;     break;
    case "else"_hash:       token_id = TokenId::Else;     break;
    case "while"_hash:      token_id = TokenId::While;    break;
    case "do"_hash:         token_id = TokenId::Do;       break;
    case "for"_hash:        token_id = TokenId::For;      break;
    case "to"_hash:         token_id = TokenId::To;       break;
    case "foreach"_hash:    token_id = TokenId::Foreach;  break;
    case "as"_hash:         token_id = TokenId::As;       break;
    case "break"_hash:      token_id = TokenId::Break;    break;
    case "continue"_hash:   token_id = TokenId::Continue; break;
    case "return"_hash:     token_id = TokenId::Return;   break;
    case "match"_hash:      token_id = TokenId::Match;    break;
    case "delay"_hash:      token_id = TokenId::Delay;    break;

    case "new"_hash:        token_id = TokenId::New;      break;

    case "none"_hash:       token_id = TokenId::None;     break;
    case "true"_hash:       token_id = TokenId::True;     break;
    case "false"_hash:      token_id = TokenId::False;    break;

    case "and"_hash:        token_id = TokenId::And;      break;
    case "or"_hash:         token_id = TokenId::Or;       break;
    case "not"_hash:        token_id = TokenId::Not;      break;

    default:                token_id = TokenId::Identifier; break;
    }
}
}
