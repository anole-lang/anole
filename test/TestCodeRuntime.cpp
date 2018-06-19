#include "CppUnitTest.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"
#include "IceObject.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;
using TYPE = IceObject::TYPE;


#define ASSERT_COUNT(COUNT)									auto node = dynamic_pointer_cast<BlockExpr>(parser.getNode(code)); \
															Assert::AreEqual(true, node->statements.size() == COUNT); \
															auto top = make_shared<Env>(nullptr); \
															for (int i = 0; i < COUNT - 1; ++i) \
																node->statements[i]->runCode(top); \
															auto obj = node->statements[COUNT - 1]->runCode(top); \
															Assert::AreEqual(true, obj->type == TYPE::LIST); \
															auto &objects = dynamic_pointer_cast<IceListObject>(obj)->objects; \
															auto iObject = objects.begin()

#define ASSERT_TYPE_PTRTYPE_VALUE(OBJTYPE, PTRTYPE, VALUE)	Assert::AreEqual(true, (*iObject)->type == OBJTYPE); \
															Assert::AreEqual(true, dynamic_pointer_cast<PTRTYPE>(*iObject)->value == VALUE); \
															++iObject;


TEST_CLASS(TestCodeRuntime)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonVariableRuntime)
	{
		std::string code = R"coldice(
@a: 1
@b: 1.1
@c: "1.1"

[a, b, c]

)coldice";

		ASSERT_COUNT(4);
		
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1.1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "1.1");
	}

	TEST_METHOD(TestCommonFunctionRuntime)
	{
		std::string code = R"coldice(
@add(a, b): a + b
@mul(a, b): (a * b)

[add(1, 2), mul(2, 3)]

)coldice";
		/*
			This is a bug that will be run as 'b[1]' as follow:
			```
				b
				[1]
			```
		*/

		ASSERT_COUNT(3);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 6);
	}

	TEST_METHOD(TestNumericRuntime)
	{
		std::string code = R"coldice(
@a: 1 + 2 - 3 + 4 - 5 + 6 - 7
@b: 2 * 3 * 4 / 4
@c: 1.1 + 1.2 + 1.3 + 1.4 - 1.5
@d: 1 * 2.3
@e: 1 * (2 - 3 / 3) - (2 + 1)

[a, b, c, d, e]

)coldice";

		ASSERT_COUNT(6);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1 + 2 - 3 + 4 - 5 + 6 - 7);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 2 * 3 * 4 / 4);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1.1 + 1.2 + 1.3 + 1.4 - 1.5);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1 * 2.3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1 * (2 - 3 / 3) - (2 + 1));
	}

	TEST_METHOD(TestStringRuntime)
	{
		std::string code = R"coldice(
@a: "test common string"
@b: "test string with escaping \""
@c: -a
@d: a + b
@e: a * 2
@f: a + a = a * 2
@g: (a = b)

[a, b, c, d, e, f, g]

)coldice";

		ASSERT_COUNT(8);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "test common string");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "test string with escaping \"");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "gnirts nommoc tset");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "test common stringtest string with escaping \"");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "test common stringtest common string");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::BOOLEAN, IceBooleanObject, true);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::BOOLEAN, IceBooleanObject, false);
	}

	TEST_METHOD(TestListRuntime)
	{

	}

	TEST_METHOD(TestDictRuntime)
	{

	}

};