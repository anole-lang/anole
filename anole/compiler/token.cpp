#include "compiler.hpp"

#include <map>
#include <utility>

using namespace std;

namespace anole
{
Token::Token(TokenType type) noexcept
  : type(type)
{
    // ...
}

Token::Token(TokenType type, String value) noexcept
  : type(type), value(move(value))
{
    // ...
}

Token::Token(Token &&other) noexcept
  : type(other.type)
  , value(std::move(other.value))
{
    // ...
}

Token::Token(const Token &other) noexcept
  : type(other.type)
  , value(other.value)
{
    // ...
}

Token &Token::operator=(Token &&other) noexcept
{
    type = other.type;
    value = move(other.value);
    return *this;
}

Token &Token::operator=(const Token &other) noexcept
{
    type = other.type;
    value = other.value;
    return *this;
}

namespace
{
Size lc_end_of_token_type = static_cast<Size>(TokenType::End);
map<String, TokenType> lc_mapping
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
    { "&",          TokenType::BAnd     },
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

Token::Token(String value) noexcept
  : value(move(value))
{
    auto find = lc_mapping.find(this->value);
    type = (find == lc_mapping.end() ? TokenType::Identifier : find->second);
}

TokenType Token::add_token_type(const String &str)
{
    auto find = lc_mapping.find(str);
    if (find == lc_mapping.end())
    {
        return lc_mapping[str] = static_cast<TokenType>(++lc_end_of_token_type);
    }
    return find->second;
}
}
