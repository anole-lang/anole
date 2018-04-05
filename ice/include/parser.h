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
public:
    Parser(std::string &text): text(text) {};
    std::vector<Token> &getTokens();
};