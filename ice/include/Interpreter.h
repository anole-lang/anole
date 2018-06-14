#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <cstring>
#include <cstdio>
#include "Token.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "Coderun.h"
#include "IceObject.h"

using std::shared_ptr;

namespace Ice
{
	class BlockExpr; // save statements

	class Interpreter
	{

	private:

		shared_ptr<Env> top;
		shared_ptr<BlockExpr> block;

		SyntaxAnalyzer syntaxAnalyzer;

	public:

		Interpreter();
		void run();
		void run(std::string);
	};
}

#endif //__INTERPRETER_H__
