#include <cctype>
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

    Token *token = nullptr;
    string value;
    while (!token)
    {
        switch (state)
        {
        case State::Begin:
            switch (last_input_)
            {
            case '$':
                token = new Token(TokenId::TEND);
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
                token = new Token(TokenId::TASSIGN);
                break;

            case ',':
                token = new Token(TokenId::TCOMMA);
                break;

            case '.':
                token = new Token(TokenId::TDOT);
                break;

            case '(':
                token = new Token(TokenId::TLPAREN);
                break;

            case ')':
                token = new Token(TokenId::TRPAREN);
                break;

            case '[':
                token = new Token(TokenId::TLBRACKET);
                break;

            case ']':
                token = new Token(TokenId::TRBRACKET);
                break;

            case '{':
                token = new Token(TokenId::TLBRACE);
                break;

            case '}':
                token = new Token(TokenId::TRBRACE);
                break;

            case '+':
                token = new Token(TokenId::TADD);
                break;

            case '-':
                token = new Token(TokenId::TSUB);
                break;

            case '*':
                token = new Token(TokenId::TMUL);
                break;

            case '/':
                token = new Token(TokenId::TDIV);
                break;

            case '%':
                token = new Token(TokenId::TMOD);
                break;

            case '&':
                token = new Token(TokenId::TBAND);
                break;

            case '|':
                token = new Token(TokenId::TBOR);
                break;

            case '^':
                token = new Token(TokenId::TBXOR);
                break;

            case '~':
                token = new Token(TokenId::TBNEG);
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
                token = new Token(TokenId::TATAT);
                break;

            default:
                return Token(TokenId::TAT);
            }
            break;

        case State::InRET:
            switch (last_input_)
            {
            case '>':
                token = new Token(TokenId::TRET);
                break;

            default:
                return Token(TokenId::TCEQ);
            }
            break;

        case State::InNot:
            switch (last_input_)
            {
            case '=':
                token = new Token(TokenId::TCNE);
                break;

            default:
                return Token(TokenId::TNOT);
            }
            break;

        case State::InCLT:
            switch (last_input_)
            {
            case '=':
                token = new Token(TokenId::TCLE);
                break;

            case '<':
                token = new Token(TokenId::TBLS);
                break;

            default:
                return Token(TokenId::TCLT);
            }
            break;

        case State::InCGT:
            switch (last_input_)
            {
            case '=':
                token = new Token(TokenId::TCGE);
                break;

            case '>':
                token = new Token(TokenId::TBRS);
                break;

            default:
                return Token(TokenId::TCGT);
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
                    return Token(TokenId::TINTEGER, value);
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
                return Token(TokenId::TDOUBLE, value);
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
                token = new Token(TokenId::TSTRING, value);
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
        token = new Token(TokenId::TEND);
    }
    return *token;
}
}
