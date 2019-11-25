#include "../src/token.cpp"
#include "../src/tokenizer.cpp"
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
        while ((token = tokenizer.next()).token_id != TokenId::End)
        {
            cout << "TokenID: " << (int)token.token_id;
            cout << " Value: " << token.value << endl;
        }
    TEST_END

    TEST_METHOD(Numeric)
        istringstream ss(R"(1 2.3)");
        Token token;
        Tokenizer tokenizer{ss};
        while ((token = tokenizer.next()).token_id != TokenId::End)
        {
            cout << "TokenID: " << (int)token.token_id;
            cout << " Value: " << token.value << endl;
        }
    TEST_END
TEST_END
