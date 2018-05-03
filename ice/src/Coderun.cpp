#include "Coderun.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "IceObject.h"

namespace Ice
{
	void runUsingStmt(std::string &name, std::shared_ptr<Env> &top)
	{
		std::ifstream fin(name + ".ice");
		SyntaxAnalyzer syntaxAnalyzer;
		if (fin.bad())
		{
			std::cout << "cannot open module " << name << std::endl;
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

	void Env::put(std::string &name, std::shared_ptr<IceObject> obj)
	{
		objects[name] = obj;
	}

	std::shared_ptr<IceObject> Env::getObject(std::string &name)
	{
		std::shared_ptr<Env> tmp = shared_from_this();
		while (tmp != nullptr)
		{
			if (tmp->objects.find(name) != tmp->objects.end())
				return tmp->objects[name];
			else
				tmp = tmp->prev;
		}
		std::cout << "cannot find " << name << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> BlockExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> returnValue = top->getReturnValue();
		for (auto stmt : statements)
		{
			stmt->runCode(top);
			if (top->getBreakStatus() || top->getContinueStatus())
			{
				return returnValue;
			}
			if (returnValue != top->getReturnValue())
			{
				returnValue = top->getReturnValue();
				top->setReturnValue(nullptr);
				return returnValue;
			}
		}
		return nullptr;
	}

	std::shared_ptr<IceObject> NoneExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceNoneObject>();
	}

	std::shared_ptr<IceObject> IntegerExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceIntegerObject>(value);
	}

	std::shared_ptr<IceObject> DoubleExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceDoubleObject>(value);
	}

	std::shared_ptr<IceObject> BooleanExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceBooleanObject>(value);
	}

	std::shared_ptr<IceObject> StringExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceStringObject>(value);
	}

	std::shared_ptr<IceObject> IdentifierExpr::runCode(std::shared_ptr<Env> &top)
	{
		return top->getObject(name);
	}

	std::shared_ptr<IceObject> MethodCallExpr::runCode(std::shared_ptr<Env> &top)
	{
		top = std::make_shared<Env>(top);

		std::shared_ptr<IceFunctionObject> func = std::dynamic_pointer_cast<IceFunctionObject>(top->getObject(id->name));
		if (arguments.size() > func->argDecls.size())
		{
			std::cout << "The number of arguments does not match" << std::endl;
			exit(0);
		}

		for (size_t i = 0; i < arguments.size(); i++) 
			func->argDecls[i]->assignment = arguments[i];

		for (auto &argDecl : func->argDecls)
			top->put(argDecl->id->name, argDecl->assignment->runCode(top));

		std::shared_ptr<IceObject> returnValue = func->block->runCode(top);

		top = top->prev;
		return returnValue;
	}

	std::shared_ptr<IceObject> UnaryOperatorExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> obj = expression->runCode(top);
		return obj->unaryOperate(op);
	}

	std::shared_ptr<IceObject> BinaryOperatorExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> lobj = lhs->runCode(top);
		std::shared_ptr<IceObject> robj = rhs->runCode(top);
		return lobj->binaryOperate(robj, op);
	}

	std::shared_ptr<IceObject> ExprStmt::runCode(std::shared_ptr<Env> &top)
	{
		return assignment->runCode(top);
	}

	std::shared_ptr<IceObject> VariableDeclarationStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> obj = assignment->runCode(top);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> FunctionDeclarationStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> obj = std::make_shared<IceFunctionObject>(argDecls, block);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> ClassDeclarationStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> obj = std::make_shared<IceClassObject>(bases, block);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> BreakStmt::runCode(std::shared_ptr<Env> &top)
	{
		top->setBreakStatus(true);
		return nullptr;
	}

	std::shared_ptr<IceObject> ContinueStmt::runCode(std::shared_ptr<Env> &top)
	{
		top->setContinueStatus(true);
		return nullptr;
	}

	std::shared_ptr<IceObject> ReturnStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> returnValue = assignment->runCode(top);
		top->setReturnValue(returnValue);
		return nullptr;
	}

	std::shared_ptr<IceObject> UsingStmt::runCode(std::shared_ptr<Env> &top)
	{
		runUsingStmt(name, top);
		return nullptr;
	}

	std::shared_ptr<IceObject> IfElseStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		if (cond->isTrue()) returnValue = blockTrue->runCode(top);
		else returnValue = blockFalse->runCode(top);
		if (returnValue != nullptr) top->setReturnValue(returnValue);
		return returnValue;
	}

	std::shared_ptr<IceObject> WhileStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		while (cond->isTrue())
		{
			returnValue = block->runCode(top);
			if (top->getBreakStatus())
			{
				top->setBreakStatus(false);
				return returnValue;
			}
			if (top->getContinueStatus())
			{
				top->setContinueStatus(false);
				cond = this->cond->runCode(top);
				continue;
			}
			if (returnValue != nullptr)
			{
				top->setReturnValue(returnValue);
				return returnValue;
			}
			cond = this->cond->runCode(top);
		}
		return returnValue;
	}

	std::shared_ptr<IceObject> DoWhileStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;

		returnValue = block->runCode(top);
		if (top->getBreakStatus())
		{
			top->setBreakStatus(false);
			return returnValue;
		}
		if (top->getContinueStatus()) top->setContinueStatus(false);
		if (returnValue != nullptr)
		{
			top->setReturnValue(returnValue);
			return returnValue;
		}

		while (cond->isTrue())
		{
			returnValue = block->runCode(top);
			if (top->getBreakStatus())
			{
				top->setBreakStatus(false);
				return returnValue;
			}
			if (top->getContinueStatus())
			{
				top->setContinueStatus(false);
				cond = this->cond->runCode(top);
				continue;
			}
			if (returnValue != nullptr)
			{
				top->setReturnValue(returnValue);
				return returnValue;
			}
			cond = this->cond->runCode(top);
		}
		return returnValue;
	}

	std::shared_ptr<IceObject> ForStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> begin = this->begin->runCode(top);
		std::shared_ptr<IceObject> end = this->end->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		while (begin->binaryOperate(end, Token::TOKEN::TCLT)->isTrue())
		{
			returnValue = block->runCode(top);
			if (top->getBreakStatus())
			{
				top->setBreakStatus(false);
				return returnValue;
			}
			if (top->getContinueStatus())
			{
				top->setContinueStatus(false);
				begin = begin->binaryOperate(std::make_shared<IceIntegerObject>(1), Token::TOKEN::TADD);
				continue;
			}
			if (returnValue != nullptr)
			{
				top->setReturnValue(returnValue);
				return returnValue;
			}
			begin = begin->binaryOperate(std::make_shared<IceIntegerObject>(1), Token::TOKEN::TADD);
		}
		return returnValue;
	}

	std::shared_ptr<IceObject> LambdaExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceFunctionObject>(argDecls, block);
	}

	std::shared_ptr<IceObject> LambdaCallExpr::runCode(std::shared_ptr<Env> &top)
	{

		top = std::make_shared<Env>(top);

		if (expressions.size() > argDecls.size())
		{
			std::cout << "The number of arguments does not match" << std::endl;
			exit(0);
		}

		std::vector<std::shared_ptr<IceObject>> argValues;

		for (size_t i = 0; i < expressions.size(); i++)
		{
			argDecls[i]->assignment = expressions[i];
		}
		for (auto &argDecl : argDecls)
		{
			top->put(argDecl->id->name, argDecl->assignment->runCode(top));
		}

		std::shared_ptr<IceObject> returnValue = block->runCode(top);

		top = top->prev;
		return returnValue;
	}

	std::shared_ptr<IceObject> NewExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceClassObject> obj = std::dynamic_pointer_cast<IceClassObject>(top->getObject(id->name));
		std::shared_ptr<Env> _top = std::make_shared<Env>(top);
		obj->block->runCode(_top);
		return std::make_shared<IceInstanceObject>(_top);
	}

	std::shared_ptr<IceObject> DotExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceInstanceObject> obj = std::dynamic_pointer_cast<IceInstanceObject>(top->getObject(left->name));
		top = obj->top;
		std::shared_ptr<IceObject> res = right->runCode(top);
		top = top->prev;
		return res;
	}

	std::shared_ptr<IceObject> DotStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceInstanceObject> obj = std::dynamic_pointer_cast<IceInstanceObject>(top->getObject(left->name));
		top = obj->top;
		std::shared_ptr<IceObject> res = right->runCode(top);
		top = top->prev;
		return res;
	}

	// build_in_function_implement

	std::shared_ptr<IceObject> InputExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::getline(std::cin, input);
		return std::make_shared<IceStringObject>(input);
	}

	std::shared_ptr<IceObject> PrintStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::cout << top->getObject(id)->toStr() << std::endl;
		return nullptr;
	}

	std::shared_ptr<IceObject> StrExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceStringObject>(top->getObject(id)->toStr());
	}

	std::shared_ptr<IceObject> ExitStmt::runCode(std::shared_ptr<Env> &top)
	{
		exit(0);
		return nullptr;
	}

	void Env::genBuildInFunction()
	{
		genInputFunction();
		genPrintFunction();
		genStrFunction();
		genExitFunction();
	}

	void Env::genInputFunction()
	{
		std::string func_name = "input";

		VariableList args;
		std::shared_ptr<BlockExpr> block = std::make_shared<BlockExpr>();

		block->statements.push_back(std::make_shared<ReturnStmt>(std::make_shared<InputExpr>()));
		
		put(func_name, std::make_shared<IceFunctionObject>(args, block));
	}

	void Env::genPrintFunction()
	{
		std::string func_name = "print";
		std::string obj_name = "to_print";

		VariableList args;
		std::shared_ptr<BlockExpr> block = std::make_shared<BlockExpr>();

		args.push_back(std::make_shared<VariableDeclarationStmt>(std::make_shared<IdentifierExpr>(obj_name), std::make_shared<NoneExpr>()));
		block->statements.push_back(std::make_shared<PrintStmt>());

		put(func_name, std::make_shared<IceFunctionObject>(args, block));
	}

	void Env::genStrFunction()
	{
		std::string func_name = "str";
		std::string obj_name = "to_str";

		VariableList args;
		std::shared_ptr<BlockExpr> block = std::make_shared<BlockExpr>();

		args.push_back(std::make_shared<VariableDeclarationStmt>(std::make_shared<IdentifierExpr>(obj_name), std::make_shared<NoneExpr>()));
		block->statements.push_back(std::make_shared<ReturnStmt>(std::make_shared<StrExpr>()));

		put(func_name, std::make_shared<IceFunctionObject>(args, block));
	}

	void Env::genExitFunction()
	{
		std::string func_name = "exit";

		VariableList args;
		std::shared_ptr<BlockExpr> block = std::make_shared<BlockExpr>();

		block->statements.push_back(std::make_shared<ExitStmt>());

		put(func_name, std::make_shared<IceFunctionObject>(args, block));
	}
}
