#include "sample-tester.hpp"
#include "tokenizer-tester.hpp"

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
