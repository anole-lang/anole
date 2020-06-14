#include <map>
#include <utility>
#include "token.hpp"

using namespace std;

namespace anole
{
Token::Token(TokenType type, string value)
  : type(type), value(move(value)) {}

Token::Token(Token &&other) noexcept
  : type(other.type)
  , value(std::move(other.value)) {}

Token::Token(const Token &other)
  : type(other.type)
  , value(other.value) {}

Token &Token::operator=(Token &&other) noexcept
{
    type = other.type;
    value = move(other.value);
    return *this;
}

Token &Token::operator=(const Token &other)
{
    type = other.type;
    value = other.value;
    return *this;
}

namespace
{
int end_of_token_type = TokenType::End;
map<string, TokenType> mapping
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

    { "new",        TokenType::New      },
    { "enum",       TokenType::Enum     },
    { "dict",       TokenType::Dict     },

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

Token::Token(std::string value)
  : type(mapping.count(value)
        ? mapping.at(value)
        : TokenType::Identifier)
  , value(move(value)) {}

TokenType Token::add_token_type(const string &str)
{
    if (!mapping.count(str))
    {
        mapping[str] = TokenType(++end_of_token_type);
    }
    return mapping[str];
}
}
