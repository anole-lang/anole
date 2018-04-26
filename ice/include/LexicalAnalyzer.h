#ifndef __LEXICAL_ANALYZER_H__
#define __LEXICAL_ANALYZER_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "Token.h"

namespace Ice
{
	class LexicalAnalyzer
	{
	private:
		std::vector<Token> tokens;
		enum class State
		{
			Begin,

			InNot,
			InCLT,
			InCGT,

			InComment,
			InInteger,
			InDouble,
			InIdentifier,
			InString,
			InStringEscaping
		};
		void analy(std::string);

	public:
		LexicalAnalyzer() {}
		std::vector<Token> &getTokens(std::string);
		std::vector<Token>::iterator cont();
	};
}

#endif //__LEXICAL_ANALYZER_H__
