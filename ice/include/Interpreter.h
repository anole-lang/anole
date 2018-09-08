#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__


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
