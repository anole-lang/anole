#include "LexicalAnalyzer.h"

namespace Ice
{
	void LexicalAnalyzer::analy(std::string line)
	{
		State state = State::Begin;
		std::string value;
		const char *reading = line.c_str();
		while (*reading)
		{
			switch (state)
			{
			case State::Begin:
				switch (*reading)
				{
				case '$':
					tokens.push_back(Token(Token::TOKEN::TEND));
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
					tokens.push_back(Token(Token::TOKEN::TASSIGN));
					break;
				case ',':
					tokens.push_back(Token(Token::TOKEN::TCOMMA));
					break;
				case '.':
					tokens.push_back(Token(Token::TOKEN::TDOT));
					break;
				case '(':
					tokens.push_back(Token(Token::TOKEN::TLPAREN));
					break;
				case ')':
					tokens.push_back(Token(Token::TOKEN::TRPAREN));
					break;
				case '[':
					tokens.push_back(Token(Token::TOKEN::TLBRACKET));
					break;
				case ']':
					tokens.push_back(Token(Token::TOKEN::TRBRACKET));
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

			case State::InAT:
				switch (*reading)
				{
				case '@':
					tokens.push_back(Token(Token::TOKEN::TATAT));
					state = State::Begin;
					break;
				default:
					tokens.push_back(Token(Token::TOKEN::TAT));
					state = State::Begin;
					reading--;
					break;
				}
				break;
			
			case State::InRET:
				switch (*reading)
				{
				case '>':
					tokens.push_back(Token(Token::TOKEN::TRET));
					state = State::Begin;
					break;
				default:
					tokens.push_back(Token(Token::TOKEN::TCEQ));
					state = State::Begin;
					reading--;
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
					tokens.push_back(Token(Token::TOKEN::TNOT));
					state = State::Begin;
					reading--;
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
				break;

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
				case '.':
					value += '.';
					state = State::InDouble;
					break;
				default:
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
					value = "";
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
					value += '\t';
					break;
				case 'f':
					value += '\f';
					break;
				default:
					value += '\\';
					value += *reading;
					break;
				}
				state = State::InString;
				break;

			case State::InComment:
				switch (*reading)
				{
				case '\n':
					state = State::Begin;
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
			reading++;
		}
	}

	std::vector<Token> &LexicalAnalyzer::getTokens(std::string line)
	{
		tokens.clear();
		analy(line + "$");
		return tokens;
	}

	std::vector<Token>::iterator LexicalAnalyzer::cont()
	{
		int offset = tokens.size() - 1;
		std::cout << ".. ";
		tokens.pop_back();

		std::string line;
		std::getline(std::cin, line);
		line += "$";

		analy(line);
		return tokens.begin() + offset;
	}
}