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
		Assert::AreEqual("text", tokens[1].value.c_str());
		Assert::AreEqual("print", tokens[4].value.c_str());
		Assert::AreEqual("text", tokens[6].value.c_str());
	}

	TEST_METHOD(TestLexerNumber)
	{
		std::string code = R"coldice(
123 123. 123.123
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(4, tokens.size());
		Assert::AreEqual("123", tokens[0].value.c_str());
		Assert::AreEqual("123.", tokens[1].value.c_str());
		Assert::AreEqual("123.123", tokens[2].value.c_str());
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
		std::string code = R"coldice(
using if else while do for to break continue return match new none true false and or not
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(19, tokens.size());
	}

	TEST_METHOD(TestLexerString)
	{
		std::string code = R"coldice(
"abcdefg" "abcdefg\"" "\"" "\\\n"
)coldice";
		auto tokens = lexer.getTokens(code);
		Assert::AreEqual<size_t>(5, tokens.size());
	}

};

