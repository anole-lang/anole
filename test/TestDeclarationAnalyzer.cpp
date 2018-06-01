#include "CppUnitTest.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;

#define ASSERT_COUNT(COUNT) Assert::AreEqual(true, std::dynamic_pointer_cast<BlockExpr>(node) != nullptr) ; Assert::AreEqual<size_t>(COUNT, std::dynamic_pointer_cast<BlockExpr>(node)->statements.size())

TEST_CLASS(TestDeclarationAnalyzer)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonDeclaration)
	{
		std::string code = R"coldice(
@_one: 1
@_one_dot_one: 1.1
@_str: "string"
)coldice";
		auto node = parser.getNode(code);

		ASSERT_COUNT(3);
	}
};