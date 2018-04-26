#include "Interpreter.h"

namespace Ice
{
	Interpreter::Interpreter()
	{
		std::cout << "    _____________________\n"
					 "   /_  ___/ _____/ _____/\n"
					 "    / /  / /    / /____      Version 0.0.1 \n"
					 " __/ /__/ /____/ /____       http://www.jusot.com/ice\n"
					 "/______/______/______/\n"
					<< std::endl;
		top = std::make_shared<Env>(nullptr);
		top->genBuildInFunction();
		block = std::make_shared<BlockExpr>();
	}

	void Interpreter::run()
	{
		while (!std::cin.eof())
		{
			std::cout << ">> ";
			auto node = syntaxAnalyzer.getNode();
			auto obj = (node == nullptr) ? nullptr : node->runCode(top);
			if (obj != nullptr) obj->show();
			block->statements.push_back(std::dynamic_pointer_cast<Stmt>(node));
		}
	}
}
