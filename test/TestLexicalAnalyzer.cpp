#include "CppUnitTest.h"
#include "LexicalAnalyzer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace Ice;

TEST_CLASS(TestLexicalAnalyzer)
{
private:

	LexicalAnalyzer lexer;

public:

	TEST_METHOD(TestLexerIdentifier)
	{
		std::string code = R"coldice(
@text: "hello world"
print(text)
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(9, tokens.size());
	}

	TEST_METHOD(TestLexerNumber)
	{
		std::string code = R"coldice(
123 123. 123.123
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(4, tokens.size());
	}

	TEST_METHOD(TestLexerOperators)
	{
		std::string code = R"coldice(
+-*/%&|^~>< >> << = != <= >= and or else
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(21, tokens.size());
	}

	TEST_METHOD(TestLexerKeywords)
	{

	}

	TEST_METHOD(TestLexerString)
	{

	}

};

