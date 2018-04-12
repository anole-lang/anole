#include "LexicalAnalyzer.h"

std::vector<Token> &LexicalAnalyzer::getTokens()
{
    tokens.clear();
    State state = State::Begin;
    const char *reading = text.c_str();
    std::string value = "";
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
            case '=':
            state = State::InIdentifier;
                tokens.push_back(Token(Token::TOKEN::TCEQ));
                break;
            case '!':
                tokens.push_back(Token(Token::TOKEN::TEXC));
                break;
            case '<':
                tokens.push_back(Token(Token::TOKEN::TCLT));
                break;
            case '>':
                tokens.push_back(Token(Token::TOKEN::TCGT));
                break;
            case ':':
                tokens.push_back(Token(Token::TOKEN::TASSIGN));
                break;
            case '@':
                tokens.push_back(Token(Token::TOKEN::TAT));
                break;
            case ',':
                tokens.push_back(Token(Token::TOKEN::TCOMMA));
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
                break;
            }
            break;
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
            default:
                if (('a' <= *reading && *reading <= 'z') || ('A' <= *reading && *reading <= 'Z') || *reading == '_')
                {
                    std::cout << "syntax error!" << std::endl;
                    exit(0);
                }
                tokens.push_back(Token(Token::TOKEN::TINTEGER, value));
                state = State::Begin;
                value = "";
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
                continue;
            }
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