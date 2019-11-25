#include <string>
#include <cassert>
#include <sstream>
#include <iostream>

#define TEST_CLASS(NAME)            static int TEST_CLASS_##NAME = []{\
                                        std::cout << "Test class " #NAME "..." << std::endl;
#define TEST_METHOD(NAME)           static int TEST_METHOD_##NAME = []{\
                                        std::cout << "Test method " #NAME "..." << std::endl;
#define TEST_END                    std::cout << std::endl; return 0; }();
#define ASSERT(EXPR)                assert(EXPR);

int main()
{
    return 0;
}
