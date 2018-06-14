#ifndef __LEXICAL_ANALYZER_H__
#define __LEXICAL_ANALYZER_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "Token.h"

using std::string;
using std::vector;

namespace Ice
{
	class LexicalAnalyzer
	{
	private:

		std::vector<Token> tokens;
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
		void analy(string);

	public:

		LexicalAnalyzer() {}
		vector<Token> &getTokens(string);
		vector<Token>::iterator cont();
	};
}

#endif //__LEXICAL_ANALYZER_H__
