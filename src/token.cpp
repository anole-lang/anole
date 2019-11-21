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
Token::Token(const Token &token)
  : tokenId(token.tokenId),
    value(token.value)
{
    // ...
}

Token::Token(std::string value) : value(value)
{
    switch (operator""_hash(value.c_str(), value.size() + 1))
    {
    case "using"_hash:      tokenId = TOKEN::TUSING;    break;
    case "if"_hash:         tokenId = TOKEN::TIF;       break;
    case "elif"_hash:       tokenId = TOKEN::TELIF;     break;
    case "else"_hash:       tokenId = TOKEN::TELSE;     break;
    case "while"_hash:      tokenId = TOKEN::TWHILE;    break;
    case "do"_hash:         tokenId = TOKEN::TDO;       break;
    case "for"_hash:        tokenId = TOKEN::TFOR;      break;
    case "to"_hash:         tokenId = TOKEN::TTO;       break;
    case "foreach"_hash:    tokenId = TOKEN::TFOREACH;  break;
    case "as"_hash:         tokenId = TOKEN::TAS;       break;
    case "break"_hash:      tokenId = TOKEN::TBREAK;    break;
    case "continue"_hash:   tokenId = TOKEN::TCONTINUE; break;
    case "return"_hash:     tokenId = TOKEN::TRETURN;   break;
    case "match"_hash:      tokenId = TOKEN::TMATCH;    break;

    case "new"_hash:        tokenId = TOKEN::TNEW;      break;

    case "none"_hash:       tokenId = TOKEN::TNONE;     break;
    case "true"_hash:       tokenId = TOKEN::TTRUE;     break;
    case "false"_hash:      tokenId = TOKEN::TFALSE;    break;

    case "and"_hash:        tokenId = TOKEN::TAND;      break;
    case "or"_hash:         tokenId = TOKEN::TOR;       break;
    case "not"_hash:        tokenId = TOKEN::TNOT;      break;

    default:                tokenId = TOKEN::TIDENTIFIER; break;
    }
}
}
