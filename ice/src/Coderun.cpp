#include "Coderun.h"
#include "Node.h"

namespace Ice
{
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

	std::shared_ptr<IceObject> BlockExpr::runCode(std::shared_ptr<Env> top)
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

	std::shared_ptr<IceObject> IntegerExpr::runCode(std::shared_ptr<Env> top)
	{
		return std::make_shared<IceIntegerObject>(value);
	}

	std::shared_ptr<IceObject> DoubleExpr::runCode(std::shared_ptr<Env> top)
	{
		return std::make_shared<IceDoubleObject>(value);
	}

	std::shared_ptr<IceObject> IdentifierExpr::runCode(std::shared_ptr<Env> top)
	{
		return top->getObject(name);
	}

	std::shared_ptr<IceObject> MethodCallExpr::runCode(std::shared_ptr<Env> top)
	{
		top = std::make_shared<Env>(top);
		return nullptr;
	}

	std::shared_ptr<IceObject> BinaryOperatorExpr::runCode(std::shared_ptr<Env> top)
	{
		std::shared_ptr<IceObject> lobj = lhs->runCode(top);
		std::shared_ptr<IceObject> robj = rhs->runCode(top);
		return lobj->binaryOperate(robj, op);
	}

	std::shared_ptr<IceObject> ExprStmt::runCode(std::shared_ptr<Env> top)
	{
		return assignment->runCode(top);
	}

	std::shared_ptr<IceObject> VariableDeclarationStmt::runCode(std::shared_ptr<Env> top)
	{
		std::shared_ptr<IceObject> obj = assignment->runCode(top);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> FunctionDeclarationStmt::runCode(std::shared_ptr<Env> top)
	{
		return nullptr;
	}

	std::shared_ptr<IceObject> ReturnStmt::runCode(std::shared_ptr<Env> top)
	{
		std::shared_ptr<IceObject> returnValue = assignment->runCode(top);
		top->setReturnValue(returnValue);
		return nullptr;
	}
}