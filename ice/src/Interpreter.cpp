#include "Interpreter.h"


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::getline;
using std::make_shared;
using std::dynamic_pointer_cast;

namespace Ice
{
	Interpreter::Interpreter()
	{
		top = make_shared<Env>(nullptr);
		top->genBuildInFunctions();
		block = make_shared<BlockExpr>();
	}

	void Interpreter::run()
	{
		cout << "    _____________________\n"
					 "   /_  ___/ _____/ _____/\n"
					 "    / /  / /    / /____      Version 0.0.1 \n"
					 " __/ /__/ /____/ /____       http://www.jusot.com/ice\n"
					 "/______/______/______/\n"
	   			  << endl;
		while (!cin.eof())
		{
			cout << ">> ";
			auto node = syntaxAnalyzer.getNode();
			auto obj = (node == nullptr) ? nullptr : node->runCode(top);
			if (obj != nullptr && obj->type != IceObject::TYPE::NONE)
			{
				obj->show();
				cout << endl;
			}
			block->statements.push_back(dynamic_pointer_cast<Stmt>(node));
		}
	}

	void Interpreter::run(string path)
	{
		ifstream fin(path);
		if (fin.bad())
		{
			cout << "cannot open file " << path << endl;
			exit(0);
		}
		string code, new_line;
		while (!fin.eof())
		{
			getline(fin, new_line);
			code += new_line + "\n";
		}
		code += "$";
		fin.close();
		auto node = syntaxAnalyzer.getNode(code);
		node->runCode(top);
	}
}
