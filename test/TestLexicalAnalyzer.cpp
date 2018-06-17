#include "CppUnitTest.h"
#include "LexicalAnalyzer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;

using TOKEN = Token::TOKEN;

#define ASSERT_COUNT(COUNT)						Assert::AreEqual<size_t>((COUNT)+1, tokens.size()); \
												auto iToken = tokens.begin()
#define ASSERT_TOKEN(TYPE)						Assert::AreEqual((int)TYPE, (int)iToken->token_id); iToken++
#define ASSERT_TOKEN_WITH_VALUE(TYPE, VALUE)	Assert::AreEqual((int)TYPE, (int)iToken->token_id); \
												Assert::AreEqual(VALUE, iToken->value.c_str()); iToken++
#define ASSERT_OVER								Assert::AreEqual((int)TOKEN::TEND, (int)iToken->token_id); iToken++; \
												Assert::AreEqual(true, iToken == tokens.end())

TEST_CLASS(TestLexicalAnalyzer)
{
private:

	LexicalAnalyzer lexer;

public:

	TEST_METHOD(TestLexerIdentifier)
	{
		std::string code = R"coldice(
test identifier
_
___
_id
id_
__id
id__
_id_
__id__
)coldice";
		auto &tokens = lexer.getTokens(code);

		ASSERT_COUNT(10);

		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "test");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "identifier");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "_");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "___");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "_id");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "id_");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "__id");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "id__");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "_id_");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "__id__");

		ASSERT_OVER;
	}

	TEST_METHOD(TestLexerNumber)
	{
		std::string code = R"coldice(
123 
123. 
123.123
)coldice";
		auto tokens = lexer.getTokens(code);
		
		ASSERT_COUNT(3);

		ASSERT_TOKEN_WITH_VALUE(TOKEN::TINTEGER, "123");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TDOUBLE, "123.");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TDOUBLE, "123.123");

		ASSERT_OVER;
	}

	TEST_METHOD(TestLexerOperators)
	{
		std::string code = R"coldice(
+-*/%&|^~>< << >> = != <= >= => and or else
)coldice";
		auto tokens = lexer.getTokens(code);
		
		ASSERT_COUNT(21);

		ASSERT_TOKEN(TOKEN::TADD);
		ASSERT_TOKEN(TOKEN::TSUB);
		ASSERT_TOKEN(TOKEN::TMUL);
		ASSERT_TOKEN(TOKEN::TDIV);
		ASSERT_TOKEN(TOKEN::TMOD);
		ASSERT_TOKEN(TOKEN::TBAND);
		ASSERT_TOKEN(TOKEN::TBOR);
		ASSERT_TOKEN(TOKEN::TBXOR);
		ASSERT_TOKEN(TOKEN::TBNEG);
		ASSERT_TOKEN(TOKEN::TCGT);
		ASSERT_TOKEN(TOKEN::TCLT);
		ASSERT_TOKEN(TOKEN::TBLS);
		ASSERT_TOKEN(TOKEN::TBRS);
		ASSERT_TOKEN(TOKEN::TCEQ);
		ASSERT_TOKEN(TOKEN::TCNE);
		ASSERT_TOKEN(TOKEN::TCLE);
		ASSERT_TOKEN(TOKEN::TCGE);
		ASSERT_TOKEN(TOKEN::TRET);
		ASSERT_TOKEN(TOKEN::TAND);
		ASSERT_TOKEN(TOKEN::TOR);
		ASSERT_TOKEN(TOKEN::TELSE);

		ASSERT_OVER;
	}

	TEST_METHOD(TestLexerKeywords)
	{
		std::string code = R"coldice(
using if else while do for to break continue 
return match new none true false and or not
)coldice";
		auto tokens = lexer.getTokens(code);
		
		ASSERT_COUNT(18);

		ASSERT_TOKEN(TOKEN::TUSING);
		ASSERT_TOKEN(TOKEN::TIF);
		ASSERT_TOKEN(TOKEN::TELSE);
		ASSERT_TOKEN(TOKEN::TWHILE);
		ASSERT_TOKEN(TOKEN::TDO);
		ASSERT_TOKEN(TOKEN::TFOR);
		ASSERT_TOKEN(TOKEN::TTO);
		ASSERT_TOKEN(TOKEN::TBREAK);
		ASSERT_TOKEN(TOKEN::TCONTINUE);
		ASSERT_TOKEN(TOKEN::TRETURN);
		ASSERT_TOKEN(TOKEN::TMATCH);
		ASSERT_TOKEN(TOKEN::TNEW);
		ASSERT_TOKEN(TOKEN::TNONE);
		ASSERT_TOKEN(TOKEN::TTRUE);
		ASSERT_TOKEN(TOKEN::TFALSE);
		ASSERT_TOKEN(TOKEN::TAND);
		ASSERT_TOKEN(TOKEN::TOR);
		ASSERT_TOKEN(TOKEN::TNOT);

		ASSERT_OVER;
	}

	TEST_METHOD(TestLexerString)
	{
		std::string code = R"coldice(
"abcdefg" 
"abcdefg\"" 
"\"" 
"\n\\\"\a\b\0\t\r\f"
)coldice";
		auto tokens = lexer.getTokens(code);
		
		ASSERT_COUNT(4);

		ASSERT_TOKEN_WITH_VALUE(TOKEN::TSTRING, "abcdefg");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TSTRING, "abcdefg\"");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TSTRING, "\"");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TSTRING, "\n\\\"\a\b\0\t\r\f");

		ASSERT_OVER;
	}

	TEST_METHOD(TestLexerComment)
	{
		std::string code = R"(
# test comments
after line # comment
)";
		auto tokens = lexer.getTokens(code);

		ASSERT_COUNT(2);

		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "after");
		ASSERT_TOKEN_WITH_VALUE(TOKEN::TIDENTIFIER, "line");

		ASSERT_OVER;
	}

};