#include <string>
#include <sstream>
#include <iostream>
#include "../source/token.cpp"
#include "../source/tokenizer.cpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

TEST_CLASS(Tokenizer)
    TEST_METHOD(Identifier)
        istringstream ss(R"(test identifier
_
___
_id
id_
_id_)");
        Token token;
        Tokenizer tokenizer{ss};
        while ((token = tokenizer.next()).tokenId != TOKEN::TEND)
        {
            cout << "TokenID: " << (int)token.tokenId;
            cout << " Value: " << token.value << endl;
        }
    TEST_END

    TEST_METHOD(Numeric)
        istringstream ss(R"(1 2.3)");
        Token token;
        Tokenizer tokenizer{ss};
        while ((token = tokenizer.next()).tokenId != TOKEN::TEND)
        {
            cout << "TokenID: " << (int)token.tokenId;
            cout << " Value: " << token.value << endl;
        }
    TEST_END
TEST_END
