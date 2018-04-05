#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include "tokens.h"

class Parser
{
private:
    std::string &text;
    std::vector<Token> tokens;
    enum class State 
    {
        Begin,
        InComment,
        InInteger,
        InIdentifier
    };

public:
    Parser(std::string &text): text(text) {};
    std::vector<Token> &getTokens();
};