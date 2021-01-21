#include "compiler.hpp"

#include <map>
#include <utility>

namespace anole
{
Token::Token(TokenType type, Location location) noexcept
  : type(type), location(location)
{
    // ...
}

Token::Token(TokenType type, String value, Location location) noexcept
  : type(type), value(std::move(value)), location(location)
{
    // ...
}

Token::Token(Token &&other) noexcept
  : type(other.type)
  , value(std::move(other.value))
  , location(std::move(other.location))
{
    // ...
}

Token::Token(const Token &other) noexcept
  : type(other.type)
  , value(other.value)
  , location(other.location)

{
    // ...
}

Token &Token::operator=(Token &&other) noexcept
{
    type = other.type;
    value = std::move(other.value);
    location = std::move(other.location);
    return *this;
}

Token &Token::operator=(const Token &other) noexcept
{
    type = other.type;
    value = other.value;
    location = other.location;
    return *this;
}

namespace
{
Size localEndOfTokenType = static_cast<Size>(TokenType::End);
std::map<String, TokenType> localMapping
{
    { "use",        TokenType::Use      },
    { "from",       TokenType::From     },
    { "prefixop",   TokenType::Prefixop },
    { "infixop",    TokenType::Infixop  },
    { "if",         TokenType::If       },
    { "elif",       TokenType::Elif     },
    { "else",       TokenType::Else     },
    { "while",      TokenType::While    },
    { "do",         TokenType::Do       },
    { "foreach",    TokenType::Foreach  },
    { "as",         TokenType::As       },
    { "break",      TokenType::Break    },
    { "continue",   TokenType::Continue },
    { "return",     TokenType::Return   },
    { "match",      TokenType::Match    },
    { "delay",      TokenType::Delay    },

    { "enum",       TokenType::Enum     },
    { "dict",       TokenType::Dict     },
    { "class",      TokenType::Class    },

    { "none",       TokenType::None     },
    { "true",       TokenType::True     },
    { "false",      TokenType::False    },

    { "and",        TokenType::And      },
    { "or",         TokenType::Or       },
    { "not",        TokenType::Not      },
    { "is",         TokenType::Is       },

    { "+",          TokenType::Add      },
    { "-",          TokenType::Sub      },
    { "*",          TokenType::Mul      },
    { "/",          TokenType::Div      },
    { "%",          TokenType::Mod      },
    { "|",          TokenType::BOr      },
    { "^",          TokenType::BXor     },
    { "~",          TokenType::BNeg     },
    { "!",          TokenType::Not      },
    { "=",          TokenType::CEQ      },
    { "!=",         TokenType::CNE      },
    { "<",          TokenType::CLT      },
    { "<=",         TokenType::CLE      },
    { ">",          TokenType::CGT      },
    { ">=",         TokenType::CGE      },
    { "=>",         TokenType::Ret      },
    { "<<",         TokenType::BLS      },
    { ">>",         TokenType::BRS      },
};
}

Token::Token(String value, Location location) noexcept
  : value(std::move(value)), location(location)
{
    auto find = localMapping.find(this->value);
    type = (find == localMapping.end() ? TokenType::Identifier : find->second);
}

TokenType Token::add_token_type(const String &str)
{
    auto find = localMapping.find(str);
    if (find == localMapping.end())
    {
        return localMapping[str] = static_cast<TokenType>(++localEndOfTokenType);
    }
    return find->second;
}
}
