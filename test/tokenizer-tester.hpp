#ifndef __TEST_TOKENIZERTESTER_HPP__
#define __TEST_TOKENIZERTESTER_HPP__

#include "../src/compiler/compiler.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace anole;

struct TokenizerTester : testing::Test
{
    stringstream ss;
    Tokenizer tokenizer{ss};

    virtual void SetUp()
    {
        tokenizer.reset();
        ss.clear();
    }
};

TEST_F(TokenizerTester, Origin)
{
    ss.str(R"(@
use from prefixop infixop if elif else
while do foreach as break continue return
match delay enum dict class none true false
identifier 0123456789 0123456789.0123456789 "String"
, . ... () [] {} : ; + - * / % & | ^ ~ << >>
and or not ! is = != < <= > >= => ?)");
    for (int type = 0; type < TokenType::End; ++type)
    {
        ASSERT_EQ(tokenizer.next().type, TokenType(type));
        if (type == TokenType::Not)
        {
            ASSERT_EQ(tokenizer.next().type, TokenType::Not);
        }
    }
}

#endif
