#include "CppUnitTest.h"
#include "SyntaxAnalyzer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;

TEST_CLASS(TestDeclarationAnalyzer)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonDeclaration)
	{
		std::string code = R"coldice(
@a: 1
)coldice";
	}
};