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


namespace Ice
{
	class Interpreter
	{

	private:

		::std::shared_ptr<Env> top;
		::std::shared_ptr<class BlockExpr> block;

		SyntaxAnalyzer syntaxAnalyzer;

	public:

		Interpreter();
		void run();
		void run(::std::string);

	};
}

#endif //__INTERPRETER_H__
