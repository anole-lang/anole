#include "CppUnitTest.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"
#include "IceObject.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Ice;
using TYPE = IceObject::TYPE;


TEST_CLASS(TestCodeRuntime)
{
private:

	SyntaxAnalyzer parser;

public:

	TEST_METHOD(TestCommonVariableDeclarationRuntime)
	{
		std::string code = R"coldice(
@a: 1
@b: 1.1
@c: "1.1"
[a, b, c]
)coldice";
		auto node = dynamic_pointer_cast<BlockExpr>(parser.getNode(code));
		auto top = make_shared<Env>(nullptr);

		for (int i = 0; i < 3; ++i)
		{
			node->statements[i]->runCode(top);
		}

		auto obj = node->statements[3]->runCode(top);

		Assert::AreEqual(true, obj->type == TYPE::LIST);
		auto &objects = dynamic_pointer_cast<IceListObject>(obj)->objects;
		
		Assert::AreEqual(true, objects[0]->type == TYPE::INT);
		Assert::AreEqual(true, dynamic_pointer_cast<IceIntegerObject>(objects[0])->value == 1);

		Assert::AreEqual(true, objects[1]->type == TYPE::DOUBLE);
		Assert::AreEqual(true, dynamic_pointer_cast<IceDoubleObject>(objects[1])->value == 1.1);

		Assert::AreEqual(true, objects[2]->type == TYPE::STRING);
		Assert::AreEqual(true, dynamic_pointer_cast<IceStringObject>(objects[2])->value == "1.1");
	}

};