#include "CppUnitTest.h"
#include "Ice.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
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


TEST_CLASS(TestBaseRuntime)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonVariableRuntime)
	{
		string code = R"coldice(
@a: 1
@b: 1.1
@c: "1.1"

a b c

)coldice";

		auto node = dynamic_pointer_cast<BlockExpr>(parser.getNode(code));
		Assert::AreEqual(true, node->statements.size() == 6); 

		auto top = make_shared<Env>(nullptr); 
		for (int i = 0; i < 3; ++i)
		{
			node->statements[i]->runCode(top);
		}

		auto obj = node->statements[3]->runCode(top); 
		Assert::AreEqual(true, obj->type == TYPE::INT);
		Assert::AreEqual(true, dynamic_pointer_cast<IceIntegerObject>(obj)->value == 1);
		
		obj = node->statements[4]->runCode(top);
		Assert::AreEqual(true, obj->type == TYPE::DOUBLE);
		Assert::AreEqual(true, dynamic_pointer_cast<IceDoubleObject>(obj)->value == 1.1);

		obj = node->statements[5]->runCode(top);
		Assert::AreEqual(true, obj->type == TYPE::STRING);
		Assert::AreEqual(true, dynamic_pointer_cast<IceStringObject>(obj)->value == "1.1");
	}

	TEST_METHOD(TestListRuntime)
	{
		string code = R"coldice(
@res: [1, 1.1, "1.1"]
res.pop_back()
res.push_back("1.1.1")

res

)coldice";
		
		ASSERT_COUNT(4);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1.1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "1.1.1");
	}

	TEST_METHOD(TestCommonFunctionRuntime)
	{
		string code = R"coldice(
@add(a, b): a + b
@mul(a, b) {
	return a * b
}

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
		string code = R"coldice(
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
		string code = R"coldice(
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

	TEST_METHOD(TestDictRuntime)
	{
		string code = R"coldice(
@a: 1
@b: 1.2
@c: "1.23"
@d: {a: 1, b: 1.2, c: "1.23", 4: [a, b, c]}

[d[a], d[b], d[c]] + d[4]

)coldice";

		ASSERT_COUNT(5);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1.2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "1.23");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 1.2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "1.23");
	}

};


TEST_CLASS(TestControlFlowRuntime)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestIfElseRuntime)
	{
		string code = R"coldice(
@pow(num, n: 2) {
	if n > 0 {
		return num * pow(num, n-1)
	} else {
		return 1
	}
}

@fib(n) {
	if n = 1 or n = 2 {
		return 1
	} else {
		return fib(n-1) + fib(n-2)
	}
}

@checkSexIsMaleOrFemale(name) {
	if name = "male" {
		return "isMale"
	} elif name = "female" {
		return "isFemale"
	} else {
		return "uncertain"
	}
}

[pow(3), pow(4, 5), fib(6), checkSexIsMaleOrFemale("male"), checkSexIsMaleOrFemale("female"), checkSexIsMaleOrFemale("cyy")]

)coldice";

		ASSERT_COUNT(4);

		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 9);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1024);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 8);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "isMale");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "isFemale");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "uncertain");
	}

	TEST_METHOD(TestWhileRuntime)
	{
		string code = R"coldice(
@pow(num, n:2) {
	@res: 1
	while n > 0 {
		@.res: res * num
		@.n: n - 1
	}
	return res
}

[pow(3), pow(4, 5)]

)coldice";

		ASSERT_COUNT(2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 9);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1024);
	}

	TEST_METHOD(TestDoWhileRuntime)
	{
		string code = R"coldice(
@test(n) {
	@test: 0
	do {
		@.test: test + 1
	} while test < n
	return test
}

[test(0), test(1), test(2)]

)coldice";

		ASSERT_COUNT(2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 2);
	}

	TEST_METHOD(TestForRuntime)
	{
		string code = R"coldice(
@res: []
for 1 to 5 as i {
	res.push_back(i)
}

res

)coldice";

		ASSERT_COUNT(3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 4);
	}

	TEST_METHOD(TestForeachRuntime)
	{
		string code = R"coldice(
@list: [1, "2", 3.4]
@res: []
foreach list as value {
	res.push_back(value)
}
res
)coldice";

		ASSERT_COUNT(4);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::STRING, IceStringObject, "2");
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::DOUBLE, IceDoubleObject, 3.4);
	}

	TEST_METHOD(TestContinueRuntime)
	{
		string code = R"coldice(
@res: []
for 1 to 5 as i {
	if i = 3 {
		continue
	} else {
		res.push_back(i)
	}
}

res

)coldice";

		ASSERT_COUNT(3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 2);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 4);
	}

	TEST_METHOD(TestBreakRuntime)
	{
		string code = R"coldice(
@res: []
for 1 to 5 as i {
	if i = 3 {
		break
	} 
	res.push_back(i)
}

res

)coldice";

		ASSERT_COUNT(3);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 1);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 2);
	}
};


TEST_CLASS(TestFunctionalRuntime)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestLambdaRuntime)
	{
		string code = R"coldice(
@add: @(a, b) {
	return a + b
}

@addFunction(): @(a, b) {
	return a + b
}

@addFuncFunction(): @() {
	return @(a, b) {
		return a + b
	}
}

@addFuncFuncFunction(): @() {
	return @() {
		return @(a, b) {
			return a + b
		}
	}
}

[	
	add(3, 4), 
	addFunction()(5, 6), 
	addFuncFunction()()(7, 8), 
	addFuncFuncFunction()()()(9, 10),
	@() {
		return @() {
			return @() {
				return @(a, b) {
					return a + b
				}
			}
		}
	}()()()(11, 12)
]

)coldice";

		ASSERT_COUNT(5);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 7);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 11);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 15);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 19);
		ASSERT_TYPE_PTRTYPE_VALUE(TYPE::INT, IceIntegerObject, 23);
	}
};