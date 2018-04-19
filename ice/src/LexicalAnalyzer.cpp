#include "LexicalAnalyzer.h"

std::vector<Token> &LexicalAnalyzer::getTokens()
{
    tokens.clear();
    State state = State::Begin;
    std::string value = "";
    const char *reading = text.c_str();
    while (*reading)
    {
        switch (state)
        {
        case State::Begin:
            switch (*reading)
            {
            case '#':
                state = State::InComment;
                break;
            case '@':
                tokens.push_back(Token(Token::TOKEN::TAT));
                break;
            case '"':
                state = State::InString;
                break;
            case ':':
                tokens.push_back(Token(Token::TOKEN::TASSIGN));
                break;
            case ',':
                tokens.push_back(Token(Token::TOKEN::TCOMMA));
                break;
            case '\\':
                tokens.push_back(Token(Token::TOKEN::TESCAPE));
                break;
            case '(':
                tokens.push_back(Token(Token::TOKEN::TLPAREN));
                break;
            case ')':
                tokens.push_back(Token(Token::TOKEN::TRPAREN));
                break;
            case '{':
                tokens.push_back(Token(Token::TOKEN::TLBRACE));
                break;
            case '}':
                tokens.push_back(Token(Token::TOKEN::TRBRACE));
                break;
            case '+':
                tokens.push_back(Token(Token::TOKEN::TADD));
                break;
            case '-':
                tokens.push_back(Token(Token::TOKEN::TSUB));
                break;
            case '*':
                tokens.push_back(Token(Token::TOKEN::TMUL));
                break;
            case '/':
                tokens.push_back(Token(Token::TOKEN::TDIV));
                break;
            case '%':
                tokens.push_back(Token(Token::TOKEN::TMOD));
                break;
            case '&':
                tokens.push_back(Token(Token::TOKEN::TBAND));
                break;
            case '|':
                tokens.push_back(Token(Token::TOKEN::TBOR));
                break;
            case '^':
                tokens.push_back(Token(Token::TOKEN::TBXOR));
                break;
            case '~':
                tokens.push_back(Token(Token::TOKEN::TBNEG));
                break;
            case '=':
                tokens.push_back(Token(Token::TOKEN::TCEQ));
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
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                state = State::InInteger;
                value += *reading;
                break;
            default:
                if (('a' <= *reading && *reading <= 'z') || ('A' <= *reading && *reading <= 'Z') || *reading == '_')
                {
                    state = State::InIdentifier;
                    value += *reading;
                }
                else if (*reading == '\"')
                {
                    state = State::InString;
                }
                break;
            }
            break;

        case State::InNot:
            switch (*reading)
            {
            case '=':
                tokens.push_back(Token(Token::TOKEN::TCNE));
                state = State::Begin;
                break;
            default:
                reading--;
                state = State::Begin;
                break;
            }
            break;

        case State::InCLT:
            switch (*reading)
            {
            case '=':
                tokens.push_back(Token(Token::TOKEN::TCLE));
                state = State::Begin;
                break;
            case '<':
                tokens.push_back(Token(Token::TOKEN::TBLS));
                state = State::Begin;
                break;
            default:
                tokens.push_back(Token(Token::TOKEN::TCLT));
                state = State::Begin;
                reading--;
                break;
            }

        case State::InCGT:
            switch (*reading)
            {
            case '=':
                tokens.push_back(Token(Token::TOKEN::TCGE));
                state = State::Begin;
                break;
            case '>':
                tokens.push_back(Token(Token::TOKEN::TBRS));
                state = State::Begin;
                break;
            default:
                tokens.push_back(Token(Token::TOKEN::TCGT));
                state = State::Begin;
                reading--;
                break;
            }

        case State::InInteger:
            switch (*reading)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                value += *reading;
                break;
            case '.':
                value += '.';
                state = State::InDouble;
                break;
            default:
                if (('a' <= *reading && *reading <= 'z') || ('A' <= *reading && *reading <= 'Z') || *reading == '_')
                {
                    std::cout << "syntax error!" << std::endl;
                    exit(0);
                }
                tokens.push_back(Token(Token::TOKEN::TINTEGER, value));
                state = State::Begin;
                value = "";
                reading--;
                break;
            }
            break;

        case State::InDouble:
            switch (*reading)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                value += *reading;
                break;
            default:
                if (('a' <= *reading && *reading <= 'z') || ('A' <= *reading && *reading <= 'Z') || *reading == '_')
                {
                    std::cout << "syntax error!" << std::endl;
                    exit(0);
                }
                tokens.push_back(Token(Token::TOKEN::TDOUBLE, value));
                state = State::Begin;
                value = "";
                reading--;
                break;
            }
            break;

        case State::InIdentifier:
            if (('a' <= *reading && *reading <= 'z') || ('A' <= *reading && *reading <= 'Z') || *reading == '_')
            {
                value += *reading;
            }
            else
            {
                tokens.push_back(Token(value));
                state = State::Begin;
                value = "";
                reading--;
            }
            break;

        case State::InString:
            switch (*reading)
            {
            case '\n':
                std::cout << "syntax error!" << std::endl;
                exit(0);
            case '\\':
                state = State::InStringEscaping;
                break;
            case '"':
                tokens.push_back(Token(Token::TOKEN::TSTRING, value));
                state = State::Begin;
                break;
            default:
                value += *reading;
                break;
            }
            break;

        case State::InStringEscaping:
            switch (*reading)
            {
            case '\n':
                std::cout << "syntax error!" << std::endl;
                exit(0);
            case 'n':
                value += '\n';
                break;
            default:
                value += '\\';
                value += *reading;
                break;
            }
            state = State::InString;
            break;

        case State::InComment:
            break;

        default:
            break;
        }
        reading++;
    }
    return tokens;
}