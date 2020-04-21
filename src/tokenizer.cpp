#include <set>
#include <cctype>
#include "error.hpp"
#include "helper.hpp"
#include "tokenizer.hpp"

using namespace std;

namespace anole
{
Tokenizer::Tokenizer(istream &in, string name_of_in)
  : cur_line_num_(1), last_line_num_(1)
  , cur_char_at_line_(0), last_char_at_line_(0)
  , input_stream_(in), name_of_in_(move(name_of_in))
  , last_input_(' ')
{
    // ...
}

void Tokenizer::cont()
{
    last_input_ = ' ';
}

void Tokenizer::reset()
{
    cur_line_num_ = last_line_num_ = 1;
    cur_char_at_line_ = last_char_at_line_ = 0;
    cur_line_.clear(), pre_line_.clear();
    last_input_ = ' ';
}

void Tokenizer::get_next_input()
{
    last_input_ = input_stream_.get();
    if (last_input_ == EOF)
    {
        return;
    }

    if (last_input_ == '\n')
    {
        ++cur_line_num_;
        cur_char_at_line_ = 0;
        pre_line_ = cur_line_;
        cur_line_.clear();
    }
    else
    {
        ++cur_char_at_line_;
        cur_line_ += last_input_;
    }
}

static const set<char> illegal_idchrs
{
    '_', '#', '@', '.', ',', ':', ';', '?', '(', ')', '[', ']', '{', '}', '"'
};
static bool is_legal_idchr(char chr)
{
    if (isspace(chr) or isdigit(chr) or isalpha(chr)
        or illegal_idchrs.count(chr))
    {
        return false;
    }
    return true;
}

Token Tokenizer::next()
{
    enum class State
    {
        Begin,

        InAT,

        InComment,
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

    last_char_at_line_ = cur_char_at_line_;
    last_line_num_ = cur_line_num_;

    Ptr<Token> token = nullptr;
    string value;
    while (!token)
    {
        switch (state)
        {
        case State::Begin:
            switch (last_input_)
            {
            case '$':
                token = make_unique<Token>(TokenType::End);
                break;

            case '#':
                state = State::InComment;
                break;

            case '@':
                state = State::InAT;
                break;

            case '"':
                state = State::InString;
                break;

            case ':':
                token = make_unique<Token>(TokenType::Colon);
                break;

            case ';':
                token = make_unique<Token>(TokenType::Semicolon);
                break;

            case ',':
                token = make_unique<Token>(TokenType::Comma);
                break;

            case '.':
                token = make_unique<Token>(TokenType::Dot);
                break;

            case '(':
                token = make_unique<Token>(TokenType::LParen);
                break;

            case ')':
                token = make_unique<Token>(TokenType::RParen);
                break;

            case '[':
                token = make_unique<Token>(TokenType::LBracket);
                break;

            case ']':
                token = make_unique<Token>(TokenType::RBracket);
                break;

            case '{':
                token = make_unique<Token>(TokenType::LBrace);
                break;

            case '}':
                token = make_unique<Token>(TokenType::RBrace);
                break;

            case '?':
                token = make_unique<Token>(TokenType::Ques);
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

        case State::InAT:
            switch (last_input_)
            {
            case '@':
                token = make_unique<Token>(TokenType::AtAt);
                break;

            default:
                return Token(TokenType::At);
            }
            break;

        case State::InInteger:
            switch (last_input_)
            {
            case '.':
                value += '.';
                state = State::InDouble;
                break;

            default:
                if (isdigit(last_input_))
                {
                    value += last_input_;
                }
                else
                {
                    return Token(TokenType::Integer, value);
                }
                break;
            }
            break;

        case State::InDouble:
            if (isdigit(last_input_))
            {
                value += last_input_;
            }
            else
            {
                return Token(TokenType::Double, value);
            }
            break;

        case State::InNormalIdentifier:
            if (isdigit(last_input_)
                or isalpha(last_input_)
                or last_input_ == '_')
            {
                value += last_input_;
            }
            else
            {
                return Token(value);
            }
            break;

        case State::InAbnormalIdentifier:
            if (is_legal_idchr(last_input_))
            {
                value += last_input_;
            }
            else
            {
                return Token(value);
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
                token = make_unique<Token>(TokenType::String, value);
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
                exit(0);

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

        case State::InComment:
            while (last_input_ != '\n')
            {
                get_next_input();
            }
            return next();

        default:
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
        token = make_unique<Token>(TokenType::End);
    }
    return move(*token);
}

std::string Tokenizer::get_err_info(const string &message)
{
    auto line = (cur_line_num_ != last_line_num_)
        ? pre_line_ : cur_line_;

    return
        info::strong(name_of_in_ + ":"
            + to_string(last_line_num_) + ":"
            + to_string(last_char_at_line_) + ": ") +
        info::warning("error: ") + message + "\n" +
        line + "\n" +
        string(last_char_at_line_ == 0 ? 0 : last_char_at_line_ - 1, ' ')
            + info::warning("^");
}
}
