#include <cctype>
#include "tokenizer.hpp"

using namespace std;

namespace ice_language
{
Tokenizer::Tokenizer(istream &in)
  : inputStream(in), lastInput(' ')
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
    while (isspace(lastInput))
    {
        lastInput = inputStream.get();
    }

    Token *token = nullptr;
    string value;
    while (!token)
    {
        switch (state)
        {
        case State::Begin:
            switch (lastInput)
            {
            case '$':
                token = new Token(TOKEN::TEND);
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
                token = new Token(TOKEN::TASSIGN);
                break;

            case ',':
                token = new Token(TOKEN::TCOMMA);
                break;

            case '.':
                token = new Token(TOKEN::TDOT);
                break;

            case '(':
                token = new Token(TOKEN::TLPAREN);
                break;

            case ')':
                token = new Token(TOKEN::TRPAREN);
                break;

            case '[':
                token = new Token(TOKEN::TLBRACKET);
                break;

            case ']':
                token = new Token(TOKEN::TRBRACKET);
                break;

            case '{':
                token = new Token(TOKEN::TLBRACE);
                break;

            case '}':
                token = new Token(TOKEN::TRBRACE);
                break;

            case '+':
                token = new Token(TOKEN::TADD);
                break;

            case '-':
                token = new Token(TOKEN::TSUB);
                break;

            case '*':
                token = new Token(TOKEN::TMUL);
                break;

            case '/':
                token = new Token(TOKEN::TDIV);
                break;

            case '%':
                token = new Token(TOKEN::TMOD);
                break;

            case '&':
                token = new Token(TOKEN::TBAND);
                break;

            case '|':
                token = new Token(TOKEN::TBOR);
                break;

            case '^':
                token = new Token(TOKEN::TBXOR);
                break;

            case '~':
                token = new Token(TOKEN::TBNEG);
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
                if (isdigit(lastInput))
                {
                    state = State::InInteger;
                    value += lastInput;
                }
                else if (lastInput == '_' || isalpha(lastInput))
                {
                    state = State::InIdentifier;
                    value += lastInput;
                }
                break;
            }
            break;

        case State::InAT:
            switch (lastInput)
            {
            case '@':
                token = new Token(TOKEN::TATAT);
                break;

            default:
                return Token(TOKEN::TAT);
            }
            break;

        case State::InRET:
            switch (lastInput)
            {
            case '>':
                token = new Token(TOKEN::TRET);
                break;

            default:
                return Token(TOKEN::TCEQ);
            }
            break;

        case State::InNot:
            switch (lastInput)
            {
            case '=':
                token = new Token(TOKEN::TCNE);
                break;

            default:
                return Token(TOKEN::TNOT);
            }
            break;

        case State::InCLT:
            switch (lastInput)
            {
            case '=':
                token = new Token(TOKEN::TCLE);
                break;

            case '<':
                token = new Token(TOKEN::TBLS);
                break;

            default:
                return Token(TOKEN::TCLT);
            }
            break;

        case State::InCGT:
            switch (lastInput)
            {
            case '=':
                token = new Token(TOKEN::TCGE);
                break;

            case '>':
                token = new Token(TOKEN::TBRS);
                break;

            default:
                return Token(TOKEN::TCGT);
            }
            break;

        case State::InInteger:
            switch (lastInput)
            {
            case '.':
                value += '.';
                state = State::InDouble;
                break;

            default:
                if (isdigit(lastInput))
                {
                    value += lastInput;
                }
                else
                {
                    return Token(TOKEN::TINTEGER, value);
                }
                break;
            }
            break;

        case State::InDouble:
            if (isdigit(lastInput))
            {
                value += lastInput;
            }
            else
            {
                return Token(TOKEN::TDOUBLE, value);
            }
            break;

        case State::InIdentifier:
            if (lastInput == '_' || isalnum(lastInput))
            {
                value += lastInput;
            }
            else
            {
                return Token(value);
            }
            break;

        case State::InString:
            switch (lastInput)
            {
            case '\n':
                cout << "syntax error!" << endl;
                exit(0);

            case '\\':
                state = State::InStringEscaping;
                break;

            case '"':
                token = new Token(TOKEN::TSTRING, value);
                break;

            default:
                value += lastInput;
                break;
            }
            break;

        case State::InStringEscaping:
            switch (lastInput)
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
                value += lastInput;
                break;
            }
            state = State::InString;
            break;

        case State::InComment:
            while (inputStream >> lastInput)
            {
                if (lastInput == '\n')
                {
                    break;
                }
            }
            return next();

        default:
            break;
        }
        if (lastInput == EOF)
        {
            break;
        }
        lastInput = inputStream.get();
    }
    if (!token)
    {
        token = new Token(TOKEN::TEND);
    }
    return *token;
}
}
