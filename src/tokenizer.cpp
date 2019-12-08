#include <cctype>
#include <memory>
#include "tokenizer.hpp"

using namespace std;

namespace ice_language
{
Tokenizer::Tokenizer(istream &in)
  : input_stream_(in), last_input_(' ')
{
    // ...
};

Token Tokenizer::next()
{
    enum class State
    {
        Begin,

        InAT,
        InNot,
        InCLT,
        InCGT,
        InRET,

        InComment,
        InInteger,
        InDouble,
        InIdentifier,
        InString,
        InStringEscaping
    };
    auto state = State::Begin;
    while (isspace(last_input_))
    {
        last_input_ = input_stream_.get();
    }

    shared_ptr<Token> token = nullptr;
    string value;
    while (!token)
    {
        switch (state)
        {
        case State::Begin:
            switch (last_input_)
            {
            case '$':
                token = make_shared<Token>(TokenId::End);
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
                token = make_shared<Token>(TokenId::Colon);
                break;

            case ',':
                token = make_shared<Token>(TokenId::Comma);
                break;

            case '.':
                token = make_shared<Token>(TokenId::Dot);
                break;

            case '(':
                token = make_shared<Token>(TokenId::LParen);
                break;

            case ')':
                token = make_shared<Token>(TokenId::RParen);
                break;

            case '[':
                token = make_shared<Token>(TokenId::LBracket);
                break;

            case ']':
                token = make_shared<Token>(TokenId::RBracket);
                break;

            case '{':
                token = make_shared<Token>(TokenId::LBrace);
                break;

            case '}':
                token = make_shared<Token>(TokenId::RBrace);
                break;

            case '+':
                token = make_shared<Token>(TokenId::Add);
                break;

            case '-':
                token = make_shared<Token>(TokenId::Sub);
                break;

            case '*':
                token = make_shared<Token>(TokenId::Mul);
                break;

            case '/':
                token = make_shared<Token>(TokenId::Div);
                break;

            case '%':
                token = make_shared<Token>(TokenId::Mod);
                break;

            case '&':
                token = make_shared<Token>(TokenId::BAnd);
                break;

            case '|':
                token = make_shared<Token>(TokenId::BOr);
                break;

            case '^':
                token = make_shared<Token>(TokenId::BXor);
                break;

            case '~':
                token = make_shared<Token>(TokenId::BNeg);
                break;

            case '?':
                token = make_shared<Token>(TokenId::Ques);
                break;

            case '=':
                state = State::InRET;
                break;

            case '!':
                state = State::InNot;
                break;

            case '<':
                state = State::InCLT;
                break;

            case '>':
                state = State::InCGT;
                break;

            default:
                if (isdigit(last_input_))
                {
                    state = State::InInteger;
                    value += last_input_;
                }
                else if (last_input_ == '_' || isalpha(last_input_))
                {
                    state = State::InIdentifier;
                    value += last_input_;
                }
                break;
            }
            break;

        case State::InAT:
            switch (last_input_)
            {
            case '@':
                token = make_shared<Token>(TokenId::AtAt);
                break;

            default:
                return Token(TokenId::At);
            }
            break;

        case State::InRET:
            switch (last_input_)
            {
            case '>':
                token = make_shared<Token>(TokenId::Ret);
                break;

            default:
                return Token(TokenId::CEQ);
            }
            break;

        case State::InNot:
            switch (last_input_)
            {
            case '=':
                token = make_shared<Token>(TokenId::CNE);
                break;

            default:
                return Token(TokenId::Not);
            }
            break;

        case State::InCLT:
            switch (last_input_)
            {
            case '=':
                token = make_shared<Token>(TokenId::CLE);
                break;

            case '<':
                token = make_shared<Token>(TokenId::BLS);
                break;

            default:
                return Token(TokenId::CLT);
            }
            break;

        case State::InCGT:
            switch (last_input_)
            {
            case '=':
                token = make_shared<Token>(TokenId::CGE);
                break;

            case '>':
                token = make_shared<Token>(TokenId::BRS);
                break;

            default:
                return Token(TokenId::CGT);
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
                    return Token(TokenId::Integer, value);
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
                return Token(TokenId::Double, value);
            }
            break;

        case State::InIdentifier:
            if (last_input_ == '_' || isalnum(last_input_))
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
                cout << "syntax error!" << endl;
                exit(0);

            case '\\':
                state = State::InStringEscaping;
                break;

            case '"':
                token = make_shared<Token>(TokenId::String, value);
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
                cout << "syntax error!" << endl;
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
            while (input_stream_ >> last_input_)
            {
                if (last_input_ == '\n')
                {
                    break;
                }
            }
            return next();

        default:
            break;
        }
        if (last_input_ == EOF)
        {
            break;
        }
        last_input_ = input_stream_.get();
    }
    if (!token)
    {
        token = make_shared<Token>(TokenId::End);
    }
    return *token;
}
}
