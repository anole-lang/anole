#include "CppUnitTest.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;


#define ASSERT_COUNT(COUNT)		Assert::AreEqual(true, std::dynamic_pointer_cast<BlockExpr>(node) != nullptr); \
								Assert::AreEqual<size_t>(COUNT, std::dynamic_pointer_cast<BlockExpr>(node)->statements.size()); \
								auto iNode = std::dynamic_pointer_cast<BlockExpr>(node)->statements.begin()

#define ASSERT_STMT_BEGIN(TYPE)	Assert::AreEqual(true, std::dynamic_pointer_cast<TYPE>(*iNode) != nullptr); \
								{ auto stmt = std::dynamic_pointer_cast<TYPE>(*iNode)

#define ASSERT_TYPE(ARG, TYPE)	Assert::AreEqual(true, std::dynamic_pointer_cast<TYPE>(ARG) != nullptr)

#define ASSERT_STMT_END			} iNode++

#define ASSERT_OVER				Assert::AreEqual(true, iNode == std::dynamic_pointer_cast<BlockExpr>(node)->statements.end())


TEST_CLASS(TestDeclarationAnalyzer)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonVariableDeclaration)
	{
		std::string code = R"coldice(
@_one: 1
@_one_dot_one: 1.1
@_str: "string"
)coldice";
		auto node = parser.getNode(code);

		ASSERT_COUNT(3);

		ASSERT_STMT_BEGIN(VariableDeclarationStmt);
		ASSERT_TYPE(stmt->id, IdentifierExpr);
		ASSERT_TYPE(stmt->assignment, IntegerExpr);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(VariableDeclarationStmt);
		ASSERT_TYPE(stmt->id, IdentifierExpr);
		ASSERT_TYPE(stmt->assignment, DoubleExpr);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(VariableDeclarationStmt);
		ASSERT_TYPE(stmt->id, IdentifierExpr);
		ASSERT_TYPE(stmt->assignment, StringExpr);
		ASSERT_STMT_END;

		ASSERT_OVER;
	}

	TEST_METHOD(TestCommonFunctionDeclaration)
	{
		std::string code = R"coldice(
@add(a, b): a + b
@pow(n): n * n
@quadratic_sum(a, b): add(pow(a), pow(b))
@fib(n) {
	if n = 0 or n = 1 { 
		return 1 
	} else { 
		return fib(n-1) + fib(n-2) 
	}
}
)coldice";
		auto node = parser.getNode(code);

		ASSERT_COUNT(4);

		ASSERT_STMT_BEGIN(FunctionDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(FunctionDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(FunctionDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(FunctionDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_OVER;
	}

	TEST_METHOD(TestCommonClassDeclaration)
	{
		std::string code = R"coldice(
@@TestClass() {
	
	@name: "origin"	

	@method() {
		return self.name
	}

	@TestClass() {
		@self.name: "TestClass"
	}
}

@instance: new TestClass()
)coldice";
		auto node = parser.getNode(code);

		ASSERT_COUNT(2);

		ASSERT_STMT_BEGIN(ClassDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_STMT_BEGIN(VariableDeclarationStmt);
		ASSERT_STMT_END;

		ASSERT_OVER;
	}

};