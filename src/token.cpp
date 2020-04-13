#include <map>
#include <utility>
#include "token.hpp"

using namespace std;

namespace ice_language
{
Token::Token(TokenType type, string value)
  : type(type), value(move(value)) {}

Token::Token(Token &&token) noexcept
  : type(token.type)
  , value(std::move(token.value)) {}

Token::Token(const Token &token)
  : type(token.type)
  , value(token.value) {}

Token &Token::operator=(const Token &token)
{
    type = token.type;
    value = token.value;
    return *this;
}

static const map<string, TokenType> mapping
{
    { "use",        TokenType::Use      },
    { "from",       TokenType::From     },
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

    { "none",       TokenType::None     },
    { "true",       TokenType::True     },
    { "false",      TokenType::False    },

    { "and",        TokenType::And      },
    { "or",         TokenType::Or       },
    { "not",        TokenType::Not      },
    { "is",         TokenType::Is       }
};

Token::Token(std::string value)
  : type(mapping.count(value)
        ? mapping.at(value)
        : TokenType::Identifier)
  , value(move(value)) {}
}
