#include "stdafx.h"
#include "CppUnitTest.h"
#include "LexicalAnalyzer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace Ice;

TEST_CLASS(TestLexicalAnalyzer)
{
public:
		
	TEST_METHOD(TestLexerIdentifier)
	{
		std::string code = "abc";
		LexicalAnalyzer lexicalAnalyzer;
		auto tokens = lexicalAnalyzer.getTokens(code);
	}

	TEST_METHOD(TestLexerNumber)
	{

	}

	TEST_METHOD(TestLexerOperators)
	{

	}

	TEST_METHOD(TestLexerKeywords)
	{

	}

	TEST_METHOD(TestLexerString)
	{

	}

};