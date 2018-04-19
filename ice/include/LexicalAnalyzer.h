#ifndef __LEXICAL_ANALYZER_H__
#define __LEXICAL_ANALYZER_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "Token.h"

class LexicalAnalyzer
{
  private:
    std::string &text;
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

  public:
    LexicalAnalyzer(std::string &text) : text(text) {}
    std::vector<Token> &getTokens();
};

#endif //__LEXICAL_ANALYZER_H__
