#include "Interpreter.h"

namespace Ice
{
	Interpreter::Interpreter()
	{
		top = std::make_shared<Env>(nullptr);
		top->genBuildInFunctions();
		block = std::make_shared<BlockExpr>();
	}

	void Interpreter::run()
	{
		std::cout << "    _____________________\n"
					 "   /_  ___/ _____/ _____/\n"
					 "    / /  / /    / /____      Version 0.0.1 \n"
					 " __/ /__/ /____/ /____       http://www.jusot.com/ice\n"
					 "/______/______/______/\n"
	   			  << std::endl;
		while (!std::cin.eof())
		{
			std::cout << ">> ";
			auto node = syntaxAnalyzer.getNode();
			auto obj = (node == nullptr) ? nullptr : node->runCode(top);
			if (obj != nullptr && obj->type != IceObject::TYPE::NONE)
			{
				obj->show();
				std::cout << std::endl;
			}
			block->statements.push_back(std::dynamic_pointer_cast<Stmt>(node));
		}
	}

	void Interpreter::run(std::string path)
	{
		std::ifstream fin(path);
		if (fin.bad())
		{
			std::cout << "cannot open file " << path << std::endl;
			exit(0);
		}
		std::string code, new_line;
		while (!fin.eof())
		{
			std::getline(fin, new_line);
			code += new_line + "\n";
		}
		code += "$";
		fin.close();
		auto node = syntaxAnalyzer.getNode(code);
		node->runCode(top);
	}
}
