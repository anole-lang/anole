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

	void Env::garbageCollect(std::string name)
	{
		if (IceObject::isInstance(objects[name]->type))
		{
			if (objects[name].use_count() == 2)
			{
				std::dynamic_pointer_cast<IceInstanceObject>(objects[name])->top->objects["self"] = nullptr;
			}
		}
	}

	void Env::put(std::string name, std::shared_ptr<IceObject> obj)
	{
		if (objects.find(name) != objects.end())
		{
			garbageCollect(name);
		}
		objects[name] = obj;
	}

	void Env::replace(std::string name, std::shared_ptr<IceObject> obj)
	{
		std::shared_ptr<Env> tmp = shared_from_this();
		while (tmp != nullptr)
		{
			if (tmp->objects.find(name) != tmp->objects.end())
			{
				tmp->garbageCollect(name);
				tmp->objects[name] = obj;
				return;
			}
			else
				tmp = tmp->prev;
		}
		objects[name] = obj;
		return;
	}

	std::shared_ptr<IceObject> Env::getObject(std::string name)
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

	void Env::garbageCollection()
	{
		for (auto iter = objects.begin(); iter != objects.end(); iter++)
		{
			if (IceObject::isInstance(iter->second->type))
			{
				if (iter->second.use_count() == 2)
					std::dynamic_pointer_cast<IceInstanceObject>(iter->second)->top->objects["self"] = nullptr;
			}
		}
	}

	std::shared_ptr<IceObject> BlockExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> returnValue = top->getReturnValue();
		for (auto stmt : statements)
		{
			stmt->runCode(top);
			if (top->getBreakStatus() || top->getContinueStatus() || returnValue != nullptr)
			{
				break;
			}
			if (returnValue != top->getReturnValue())
			{
				returnValue = top->getReturnValue();
				top->setReturnValue(nullptr);
				break;
			}
		}
		top->garbageCollection();
		return returnValue;
	}

	std::shared_ptr<IceObject> NoneExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return std::make_shared<IceNoneObject>();
	}

	std::shared_ptr<IceObject> IntegerExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return std::make_shared<IceIntegerObject>(value);
	}

	std::shared_ptr<IceObject> DoubleExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return std::make_shared<IceDoubleObject>(value);
	}

	std::shared_ptr<IceObject> BooleanExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return std::make_shared<IceBooleanObject>(value);
	}

	std::shared_ptr<IceObject> StringExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceStringObject> obj = std::make_shared<IceStringObject>(value);
		obj->top->put("self", obj);
		return obj;
	}

	std::shared_ptr<IceObject> IdentifierExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return top->getObject(name);
	}

	std::shared_ptr<IceObject> MethodCallExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type == IceObject::TYPE::BUILT_IN_FUNCTION)
		{
			Objects objects;
			for (auto &argument : arguments)
				objects.push_back(argument->runCode((normal_top == nullptr) ? (top) : (normal_top)));
			return std::dynamic_pointer_cast<IceBuiltInFunctionObject>(_obj)->func(objects);
		}
		else if (_obj->type != IceObject::TYPE::FUNCTION)
		{
			std::cout << "call need function type" << std::endl;
			exit(0);
		}

		std::shared_ptr<IceFunctionObject> func = std::dynamic_pointer_cast<IceFunctionObject>(_obj);
		std::shared_ptr<Env> _top = std::make_shared<Env>(top);
		if (arguments.size() > func->argDecls.size())
		{
			std::cout << "The number of arguments does not match" << std::endl;
			exit(0);
		}

		for (size_t i = 0; i < arguments.size(); i++) 
			func->argDecls[i]->assignment = arguments[i];

		for (auto &argDecl : func->argDecls)
			_top->put(argDecl->id->name, argDecl->assignment->runCode((normal_top == nullptr) ? (_top) : (normal_top)));

		std::shared_ptr<IceObject> returnValue = func->block->runCode(_top);
		return returnValue;
	}

	std::shared_ptr<IceObject> UnaryOperatorExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = expression->runCode(top);
		return obj->unaryOperate(op);
	}

	std::shared_ptr<IceObject> BinaryOperatorExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> lobj = lhs->runCode(top);
		std::shared_ptr<IceObject> robj = rhs->runCode(top);
		return lobj->binaryOperate(robj, op);
	}

	std::shared_ptr<IceObject> ExprStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return assignment->runCode(top);
	}

	std::shared_ptr<IceObject> VariableDeclarationStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> VariableAssignStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = assignment->runCode(top);
		top->replace(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> FunctionDeclarationStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = std::make_shared<IceFunctionObject>(argDecls, block);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> ClassDeclarationStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = std::make_shared<IceClassObject>(bases, block);
		top->put(id->name, obj);
		return nullptr;
	}

	std::shared_ptr<IceObject> BreakStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		top->setBreakStatus(true);
		return nullptr;
	}

	std::shared_ptr<IceObject> ContinueStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		top->setContinueStatus(true);
		return nullptr;
	}

	std::shared_ptr<IceObject> ReturnStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> returnValue = assignment->runCode(top);
		top->setReturnValue(returnValue);
		return nullptr;
	}

	std::shared_ptr<IceObject> UsingStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		runUsingStmt(name, top);
		return nullptr;
	}

	std::shared_ptr<IceObject> IfElseStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		if (cond->isTrue())
		{
			std::shared_ptr<Env> _top = std::make_shared<Env>(top);
			returnValue = blockTrue->runCode(_top);
			if (_top->getBreakStatus())
			{
				top->setBreakStatus(true);
			}
			if (_top->getContinueStatus())
			{
				top->setContinueStatus(true);
			}
		}
		else
		{
			std::shared_ptr<Env> _top = std::make_shared<Env>(top);
			returnValue = blockFalse->runCode(_top);
			if (_top->getBreakStatus())
			{
				top->setBreakStatus(true);
			}
			if (_top->getContinueStatus())
			{
				top->setContinueStatus(true);
			}
		}
		if (returnValue != nullptr) top->setReturnValue(returnValue);
		return returnValue;
	}

	std::shared_ptr<IceObject> WhileStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		std::shared_ptr<Env> _top = std::make_shared<Env>(top);

		while (cond->isTrue())
		{
			returnValue = block->runCode(_top);
			if (_top->getBreakStatus())
			{
				return returnValue;
			}
			if (_top->getContinueStatus())
			{
				_top->setContinueStatus(false);
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

	std::shared_ptr<IceObject> DoWhileStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> cond = this->cond->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;
		std::shared_ptr<Env> _top = std::make_shared<Env>(top);

		returnValue = block->runCode(_top);
		if (_top->getBreakStatus())
		{
			return returnValue;
		}
		if (_top->getContinueStatus()) 
		{
			_top->setContinueStatus(false);
		}
		if (returnValue != nullptr)
		{
			top->setReturnValue(returnValue);
			return returnValue;
		}

		while (cond->isTrue())
		{
			returnValue = block->runCode(_top);
			if (_top->getBreakStatus())
			{
				return returnValue;
			}
			if (_top->getContinueStatus())
			{
				_top->setContinueStatus(false);
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

	std::shared_ptr<IceObject> ForStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> begin = this->begin->runCode(top);
		std::shared_ptr<IceObject> end = this->end->runCode(top);
		std::shared_ptr<IceObject> returnValue = nullptr;

		std::shared_ptr<Env> _top = std::make_shared<Env>(top);

		while (begin->binaryOperate(end, Token::TOKEN::TCLT)->isTrue())
		{
			returnValue = block->runCode(_top);
			if (_top->getBreakStatus())
			{
				return returnValue;
			}
			if (_top->getContinueStatus())
			{
				_top->setContinueStatus(false);
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

	std::shared_ptr<IceObject> LambdaExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		return std::make_shared<IceFunctionObject>(argDecls, block);
	}

	std::shared_ptr<IceObject> LambdaCallExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{

		std::shared_ptr<Env> _top = std::make_shared<Env>(top);

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
			_top->put(argDecl->id->name, argDecl->assignment->runCode(_top));
		}

		std::shared_ptr<IceObject> returnValue = block->runCode(_top);
		return returnValue;
	}

	std::shared_ptr<IceObject> NewExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceClassObject> class_obj = std::dynamic_pointer_cast<IceClassObject>(top->getObject(id->name));
		std::shared_ptr<IceInstanceObject> ins_obj = std::make_shared<IceInstanceObject>(top);

		for (auto &base : class_obj->bases)
		{
			std::dynamic_pointer_cast<IceClassObject>(top->getObject(base->name))->block->runCode(ins_obj->top);
			if (ins_obj->top->getObject(base->name)->type == IceObject::TYPE::FUNCTION)
			{
				std::shared_ptr<MethodCallExpr> call = std::make_shared<MethodCallExpr>(std::make_shared<IdentifierExpr>(base->name), arguments);
				call->runCode(ins_obj->top);
			}
		}

		class_obj->block->runCode(ins_obj->top);
		ins_obj->top->put("self", ins_obj);

		if (ins_obj->top->getObject(id->name)->type == IceObject::TYPE::FUNCTION)
		{
			std::shared_ptr<MethodCallExpr> call = std::make_shared<MethodCallExpr>(std::make_shared<IdentifierExpr>(id->name), arguments);
			call->runCode(ins_obj->top);
		}
		return ins_obj;
	}

	std::shared_ptr<IceObject> DotExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		if (normal_top == nullptr)
		{
			std::shared_ptr<IceObject> _obj = left->runCode(top, top);
			if (!IceObject::isInstance(_obj->type))
			{
				std::cout << "it doesn't support for '.'" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceInstanceObject> obj = std::dynamic_pointer_cast<IceInstanceObject>(_obj);
			std::shared_ptr<IceObject> res = right->runCode(obj->top, top);
			return res;
		}

		std::shared_ptr<IceObject> _obj = left->runCode(top, normal_top);
		if (!IceObject::isInstance(_obj->type)) 
		{
			std::cout << "it doesn't support for '.'" << std::endl;
			exit(0);
		}

		std::shared_ptr<IceInstanceObject> obj = std::dynamic_pointer_cast<IceInstanceObject>(_obj);
		std::shared_ptr<IceObject> res = right->runCode(obj->top, normal_top);
		return res;
	}

	std::shared_ptr<IceObject> DotStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<Env> _top = top;
		for (auto &expression : expressions)
		{
			std::shared_ptr<IceObject> _obj = expression->runCode(_top);
			if (!IceObject::isInstance(_obj->type))
			{
				std::cout << "it doesn't '.' operator" << std::endl;
				exit(0);
			}
			
			std::shared_ptr<IceInstanceObject> obj = std::dynamic_pointer_cast<IceInstanceObject>(_obj);
			_top = obj->top;
		}
		to_run->runCode(_top, top);
		return nullptr;
	}

	std::shared_ptr<IceObject> EnumExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceInstanceObject> obj = std::make_shared<IceInstanceObject>(top);;

		long i = 0;
		for (auto &enumerator : enumerators)
			obj->top->put(enumerator->name, std::make_shared<IceIntegerObject>(i++));

		obj->top->put("self", obj);

		return obj;
	}

	std::shared_ptr<IceObject> MatchExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> obj = expression->runCode(top);
		for (size_t i = 0; i < mat_expressions.size(); i++)
		{
			if (mat_expressions[i]->runCode(top)->binaryOperate(obj, Token::TOKEN::TCEQ)->isTrue())
			{
				return ret_expressions[i]->runCode(top);
			}
		}
		return else_expression->runCode(top);
	}

	std::shared_ptr<IceObject> ListExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceListObject> obj = std::make_shared<IceListObject>();
		for (auto &expression : expressions)
		{
			obj->objects.push_back(expression->runCode(top));
		}
		obj->top->put("self", obj);
		return obj;
	}

	std::shared_ptr<IceObject> IndexExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type != IceObject::TYPE::LIST && _obj->type != IceObject::TYPE::STRING && _obj->type != IceObject::TYPE::DICT)
		{
			std::cout << "it doesn't support for []" << std::endl;
			exit(0);
		}

		if (_obj->type == IceObject::TYPE::LIST) 
		{
			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
		else if (_obj->type == IceObject::TYPE::STRING)
		{
			std::shared_ptr<IceStringObject> obj = std::dynamic_pointer_cast<IceStringObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
		else
		{
			std::shared_ptr<IceDictObject> obj = std::dynamic_pointer_cast<IceDictObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
	}

	std::shared_ptr<IceObject> IndexStmt::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		std::shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type != IceObject::TYPE::LIST && _obj->type != IceObject::TYPE::DICT)
		{
			std::cout << "it doesn't support for []" << std::endl;
			exit(0);
		}

		if (_obj->type == IceObject::TYPE::LIST)
		{
			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(_obj);
			std::shared_ptr<IceObject> index = this->index->runCode((normal_top == nullptr) ? (top) : (normal_top));
			std::shared_ptr<IceObject> assignment = this->assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
			obj->setByIndex(index, assignment);
		}
		else
		{
			std::shared_ptr<IceDictObject> obj = std::dynamic_pointer_cast<IceDictObject>(_obj);
			std::shared_ptr<IceObject> index = this->index->runCode((normal_top == nullptr) ? (top) : (normal_top));
			std::shared_ptr<IceObject> assignment = this->assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
			obj->setByIndex(index, assignment);
		}
		return nullptr;
	}

	std::shared_ptr<IceObject> DictExpr::runCode(std::shared_ptr<Env> &top, std::shared_ptr<Env> normal_top)
	{
		if (keys.size() != values.size())
		{
			std::cout << "one key, one value" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceDictObject> obj = std::make_shared<IceDictObject>();
		for (size_t i = 0; i < keys.size(); i++)
		{
			obj->setByIndex(keys[i]->runCode(top), values[i]->runCode(top));
		}
		obj->top->put("self", obj);
		return obj;
	}

	// Built_in functions implement

	void Env::genBuildInFunctions()
	{
		put("input", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size())
			{
				std::cout << "input() need no arguments" << std::endl;
				exit(0);
			}
			std::string input;
			std::getline(std::cin, input);
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceStringObject>(input));
		}));

		put("print", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "print() need 1 argument but get others" << std::endl;
				exit(0);
			}
			std::cout << objects[0]->toStr() << std::endl;
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceNoneObject>());
		}));

		put("str", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "str() need 1 argument but get others" << std::endl;
				exit(0);
			}
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceStringObject>(objects[0]->toStr()));
		}));

		put("exit", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size())
			{
				if (objects.size() == 1)
				{
					if (objects[0]->type == IceObject::TYPE::INT)
					{
						exit(std::dynamic_pointer_cast<IceIntegerObject>(objects[0])->value);
					}
				}
				else
				{
					std::cout << "exit() need no arguments" << std::endl;
					exit(0);
				}
			}
			exit(0);
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceNoneObject>());
		}));


		put("len", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "len() need 1 argument but get others" << std::endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING && objects[0]->type != IceObject::TYPE::LIST)
			{
				std::cout << "len() need string object or list object but get other types" << std::endl;
				exit(0);
			}
			if (objects[0]->type == IceObject::TYPE::STRING)
			{
				return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceIntegerObject>(std::dynamic_pointer_cast<IceStringObject>(objects[0])->value.length()));
			}
			else
			{
				return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceIntegerObject>(std::dynamic_pointer_cast<IceListObject>(objects[0])->objects.size()));
			}
		}));

		put("int", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "int() need 1 arguments but get others" << std::endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING)
			{
				std::cout << "int() need string object" << std::endl;
				exit(0);
			}
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceIntegerObject>(std::atoi(std::dynamic_pointer_cast<IceStringObject>(objects[0])->value.c_str())));
		}));

		put("float", std::make_shared<IceBuiltInFunctionObject>([](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "float() need 1 arguments but get others" << std::endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING)
			{
				std::cout << "float() need string object" << std::endl;
				exit(0);
			}
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceDoubleObject>(std::atof(std::dynamic_pointer_cast<IceStringObject>(objects[0])->value.c_str())));
		}));
	}
}
