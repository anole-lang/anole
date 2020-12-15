#include "compiler.hpp"

#include "../base.hpp"

#include <set>
#include <cctype>
#include <optional>

using namespace std;

namespace anole
{
Tokenizer::Tokenizer(istream &input, String name_of_input) noexcept
  : cur_location_(1, 0), last_location_(1, 0)
  , input_(input), name_of_input_(move(name_of_input))
  , last_input_(' ')
{
    // ...
}

void Tokenizer::resume()
{
    last_input_ = ' ';
}

void Tokenizer::reset()
{
    cur_location_.first = last_location_.first = 1;
    cur_location_.second = last_location_.second = 0;
    cur_line_.clear(), last_line_.clear();
    last_input_ = ' ';
}

void Tokenizer::get_next_input()
{
    last_input_ = input_.get();
    if (last_input_ == EOF)
    {
        return;
    }

    if (last_input_ == '\n')
    {
        ++cur_location_.first;
        cur_location_.second = 0;
        last_line_ = cur_line_;
        cur_line_.clear();
    }
    else
    {
        ++cur_location_.second;
        cur_line_ += last_input_;
    }
}

namespace
{
const set<char> illegal_idchrs
{
    '_', '@', '#', '$', '.', ',', ':', ';',
    '?', '(', ')', '[', ']', '{', '}', '"'
};
bool is_legal_idchr(char chr)
{
    return !(isspace(chr)
        || isdigit(chr)
        || isalpha(chr)
        || illegal_idchrs.count(chr)
    );
}
}

Token Tokenizer::next_token()
{
    enum class State
    {
        Begin,

        InDot,
        InDoot,

        InLineComment,
        InBlockComment,

        InInteger,
        InDouble,
        InNormalIdentifier,
        InAbnormalIdentifier,
        InString,
        InStringEscaping
    };

    auto state = State::Begin;
    while (isspace(last_input_))
    {
        get_next_input();
    }

    last_location_ = cur_location_;

    optional<Token> token;
    String value;
    while (!token)
    {
        switch (state)
        {
        case State::Begin:
            switch (last_input_)
            {
            case '#':
                state = State::InLineComment;
                break;

            case '$':
                state = State::InBlockComment;
                break;

            case '@':
                token = Token(TokenType::At, last_location_);
                break;

            case '"':
                state = State::InString;
                break;

            case ':':
                token = Token(TokenType::Colon, last_location_);
                break;

            case ';':
                token = Token(TokenType::Semicolon, last_location_);
                break;

            case ',':
                token = Token(TokenType::Comma, last_location_);
                break;

            case '.':
                state = State::InDot;
                break;

            case '(':
                token = Token(TokenType::LParen, last_location_);
                break;

            case ')':
                token = Token(TokenType::RParen, last_location_);
                break;

            case '[':
                token = Token(TokenType::LBracket, last_location_);
                break;

            case ']':
                token = Token(TokenType::RBracket, last_location_);
                break;

            case '{':
                token = Token(TokenType::LBrace, last_location_);
                break;

            case '}':
                token = Token(TokenType::RBrace, last_location_);
                break;

            case '?':
                token = Token(TokenType::Ques, last_location_);
                break;

            default:
                if (isdigit(last_input_))
                {
                    state = State::InInteger;
                    value += last_input_;
                }
                else if (isalpha(last_input_) || last_input_ == '_')
                {
                    state = State::InNormalIdentifier;
                    value += last_input_;
                }
                else if (is_legal_idchr(last_input_))
                {
                    state = State::InAbnormalIdentifier;
                    value += last_input_;
                }
                break;
            }
            break;

        case State::InDot:
            if (last_input_ == '.')
            {
                state = State::InDoot;
            }
            else
            {
                return Token(TokenType::Dot, last_location_);
            }
            break;

        case State::InDoot:
            if (last_input_ == '.')
            {
                token = Token(TokenType::Dooot, last_location_);
            }
            else
            {
                throw CompileError("unexpected \"..\"");
            }
            break;

        case State::InLineComment:
            while (last_input_ != '\n')
            {
                get_next_input();
            }
            get_next_input();
            return next_token();

        case State::InBlockComment:
            while (last_input_ != '$')
            {
                get_next_input();
            }
            get_next_input();
            return next_token();

        case State::InInteger:
            if (last_input_ == '.')
            {
                value += '.';
                state = State::InDouble;
            }
            else
            {
                if (isdigit(last_input_))
                {
                    value += last_input_;
                }
                else
                {
                    return Token(TokenType::Integer, value, last_location_);
                }
            }
            break;

        case State::InDouble:
            if (isdigit(last_input_))
            {
                value += last_input_;
            }
            else
            {
                return Token(TokenType::Double, value, last_location_);
            }
            break;

        case State::InNormalIdentifier:
            if (isdigit(last_input_)
                || isalpha(last_input_)
                || last_input_ == '_')
            {
                value += last_input_;
            }
            else
            {
                return Token(value, last_location_);
            }
            break;

        case State::InAbnormalIdentifier:
            if (is_legal_idchr(last_input_))
            {
                value += last_input_;
            }
            else
            {
                return Token(value, last_location_);
            }
            break;

        case State::InString:
            switch (last_input_)
            {
            case '\n':
                throw CompileError("expected \"");

            case '\\':
                state = State::InStringEscaping;
                break;

            case '"':
                token = Token(TokenType::String, value, last_location_);
                break;

            default:
                value += last_input_;
                break;
            }
            break;

        case State::InStringEscaping:
            switch (last_input_)
            {
            case '\n':
                throw CompileError("expected \"");

            case 'n':
                value += '\n';
                break;

            case '\\':
                value += '\\';
                break;

            case '\"':
                value += '\"';
                break;

            case 'a':
                value += '\a';
                break;

            case 'b':
                value += '\b';
                break;

            case '0':
                value += '\0';
                break;

            case 't':
                value += '\t';
                break;

            case 'r':
                value += '\r';
                break;

            case 'f':
                value += '\f';
                break;

            default:
                value += '\\';
                value += last_input_;
                break;
            }
            state = State::InString;
            break;
        }

        if (last_input_ == EOF)
        {
            break;
        }
        get_next_input();
    }
    if (!token)
    {
        return Token(TokenType::End, last_location_);
    }
    return *token;
}

String Tokenizer::get_err_info(const String &message)
{
    auto line = (cur_location_.first != last_location_.first)
        ? last_line_ : cur_line_
    ;

    auto line_num = last_location_.first,
         char_at_line = last_location_.second
    ;

    return
        info::strong(name_of_input_ + ":"
            + to_string(line_num) + ":"
            + to_string(char_at_line) + ": ") +
        info::warning("error: ") + message + "\n" +
        line + "\n" +
        String(char_at_line == 0 ? 0 : char_at_line - 1, ' ')
            + info::warning("^")
    ;
}
}
