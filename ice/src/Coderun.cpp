#include "Coderun.h"
#include "Node.h"

namespace Ice
{
	void Env::put(std::string &name, std::shared_ptr<IceObject> obj)
	{
		objects[name] = obj;
	}

	void Env::put(std::string &name, std::shared_ptr<FunctionDeclarationStmt> func)
	{
		functions[name] = func;
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
		std::cout << "cannot find var " << name << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<FunctionDeclarationStmt> Env::getFunction(std::string &name)
	{
		std::shared_ptr<Env> tmp = shared_from_this();
		while (tmp != nullptr)
		{
			if (tmp->functions.find(name) != tmp->functions.end())
				return tmp->functions[name];
			else
				tmp = tmp->prev;
		}
		std::cout << "cannot find function " << name << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> BlockExpr::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> returnValue = top->getReturnValue();
		for (auto stmt : statements)
		{
			stmt->runCode(top);
			if (returnValue != top->getReturnValue())
			{
				returnValue = top->getReturnValue();
				top->setReturnValue(nullptr);
				return returnValue;
			}
		}
		return nullptr;
	}

	std::shared_ptr<IceObject> IntegerExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceIntegerObject>(value);
	}

	std::shared_ptr<IceObject> DoubleExpr::runCode(std::shared_ptr<Env> &top)
	{
		return std::make_shared<IceDoubleObject>(value);
	}

	std::shared_ptr<IceObject> IdentifierExpr::runCode(std::shared_ptr<Env> &top)
	{
		return top->getObject(name);
	}

	std::shared_ptr<IceObject> MethodCallExpr::runCode(std::shared_ptr<Env> &top)
	{
		top = std::make_shared<Env>(top);
		std::shared_ptr<FunctionDeclarationStmt> func = top->getFunction(id->name);
		if (arguments.size() != func->arguments.size())
		{
			std::cout << "The number of arguments does not match" << std::endl;
			exit(0);
		}
		std::vector<std::shared_ptr<IceObject>> argValues;
		for (auto argument : arguments)
		{
			argValues.push_back(argument->runCode(top));
		}
		for (size_t i = 0; i < this->arguments.size(); i++)
		{
			top->put(func->arguments[i]->name, argValues[i]);
		}
		std::shared_ptr<IceObject> returnValue = func->block->runCode(top);
		top = top->prev;
		return returnValue;
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
		top->put(id->name, shared_from_this());
		return nullptr;
	}

	std::shared_ptr<IceObject> ReturnStmt::runCode(std::shared_ptr<Env> &top)
	{
		std::shared_ptr<IceObject> returnValue = assignment->runCode(top);
		top->setReturnValue(returnValue);
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
		if (returnValue != nullptr)
		{
			top->setReturnValue(returnValue);
			return returnValue;
		}

		while (cond->isTrue())
		{
			returnValue = block->runCode(top);
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
			if (returnValue != nullptr)
			{
				top->setReturnValue(returnValue);
				return returnValue;
			}
			begin = begin->binaryOperate(std::make_shared<IceIntegerObject>(1), Token::TOKEN::TADD);
		}
		return returnValue;
	}

	// build_in_function_implement

	std::shared_ptr<IceObject> PrintStmt::runCode(std::shared_ptr<Env> &top)
	{
		top->getObject(id)->show();
		return nullptr;
	}

	void Env::genBuildInFunction()
	{
		genPrintFunction();
	}

	void Env::genPrintFunction()
	{
		std::string func_name = "print";
		std::string obj_name = "to_print";

		VariableList args;
		std::shared_ptr<BlockExpr> block = std::make_shared<BlockExpr>();
		args.push_back(std::make_shared<IdentifierExpr>(obj_name));
		block->statements.push_back(std::make_shared<PrintStmt>());
		put(func_name, std::make_shared<FunctionDeclarationStmt>(std::make_shared<IdentifierExpr>(func_name), args, block));
	}
}
