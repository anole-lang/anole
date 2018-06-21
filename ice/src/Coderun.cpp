#include "Coderun.h"
#include "Node.h"
#include "SyntaxAnalyzer.h"
#include "IceObject.h"
#include "ctime"

using std::cin;
using std::cout;
using std::endl;
using std::atoi;
using std::atof;
using std::getline;
using std::ifstream;
using std::vector;
using std::make_shared;
using std::dynamic_pointer_cast;

using TOKEN = Ice::Token::TOKEN;

namespace Ice
{
	void runUsingStmt(string &name, shared_ptr<Env> &top)
	{
		ifstream fin(name + ".ice");
		SyntaxAnalyzer syntaxAnalyzer;
		if (fin.bad())
		{
			cout << "cannot open module " << name << endl;
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

	void Env::garbageCollect(string name)
	{
		if (objects[name]->type == IceObject::TYPE::INSTANCE)
		{
			if (objects[name].use_count() == 2)
			{
				dynamic_pointer_cast<IceInstanceObject>(objects[name])->top->objects["self"] = nullptr;
			}
		}
	}

	void Env::put(string name, shared_ptr<IceObject> obj)
	{
		if (objects.find(name) != objects.end())
		{
			garbageCollect(name);
		}
		objects[name] = obj;
	}

	void Env::replace(string name, shared_ptr<IceObject> obj)
	{
		shared_ptr<Env> tmp = shared_from_this();
		while (tmp != nullptr)
		{
			if (tmp->objects.find(name) != tmp->objects.end())
			{
				tmp->garbageCollect(name);
				tmp->objects[name] = obj;
				return;
			}
			else
			{
				tmp = tmp->prev;
			}
		}
		objects[name] = obj;
		return;
	}

	shared_ptr<IceObject> Env::getObject(string name)
	{
		shared_ptr<Env> tmp = shared_from_this();
		while (tmp != nullptr)
		{
			if (tmp->objects.find(name) != tmp->objects.end())
			{
				return tmp->objects[name];
			}
			else
			{
				tmp = tmp->prev;
			}
		}
		cout << "cannot find " << name << endl;
		exit(0);
		return nullptr;
	}

	void Env::garbageCollection()
	{
		for (auto iter = objects.begin(); iter != objects.end(); iter++)
		{
			if (iter->second->type == IceObject::TYPE::INSTANCE)
			{
				if (iter->second.use_count() == 2)
				{
					dynamic_pointer_cast<IceInstanceObject>(iter->second)->top->objects["self"] = nullptr;
				}
			}
		}
	}

	shared_ptr<IceObject> BlockExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> returnValue = top->getReturnValue();
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
		return returnValue;
	}

	shared_ptr<IceObject> NoneExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return make_shared<IceNoneObject>();
	}

	shared_ptr<IceObject> IntegerExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return make_shared<IceIntegerObject>(value);
	}

	shared_ptr<IceObject> DoubleExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return make_shared<IceDoubleObject>(value);
	}

	shared_ptr<IceObject> BooleanExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return make_shared<IceBooleanObject>(value);
	}

	shared_ptr<IceObject> StringExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceStringObject> obj = make_shared<IceStringObject>(value);
		return obj;
	}

	shared_ptr<IceObject> IdentifierExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return top->getObject(name);
	}

	shared_ptr<IceObject> MethodCallExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type == IceObject::TYPE::BUILT_IN_FUNCTION)
		{
			Objects objects;
			for (auto &argument : arguments)
			{
				objects.push_back(argument->runCode((normal_top == nullptr) ? (top) : (normal_top)));
			}
			return dynamic_pointer_cast<IceBuiltInFunctionObject>(_obj)->func(objects);
		}
		else if (_obj->type != IceObject::TYPE::FUNCTION)
		{
			cout << "call need function type" << endl;
			exit(0);
		}

		shared_ptr<IceFunctionObject> func = dynamic_pointer_cast<IceFunctionObject>(_obj);
		shared_ptr<Env> _top = make_shared<Env>(top);
		if (arguments.size() > func->argDecls.size())
		{
			cout << "The number of arguments does not match" << endl;
			exit(0);
		}

		for (size_t i = 0; i < arguments.size(); i++)
		{
			func->argDecls[i]->assignment = arguments[i];
		}

		for (auto &argDecl : func->argDecls)
		{
			_top->put(argDecl->id->name, argDecl->assignment->runCode((normal_top == nullptr) ? (_top) : (normal_top)));
		}

		shared_ptr<IceObject> returnValue = func->block->runCode(_top);
		_top->garbageCollection();
		return returnValue;
	}

	shared_ptr<IceObject> UnaryOperatorExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = expression->runCode(top);
		return obj->unaryOperate(op);
	}

	shared_ptr<IceObject> BinaryOperatorExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> lobj = lhs->runCode(top);
		shared_ptr<IceObject> robj = rhs->runCode(top);
		return lobj->binaryOperate(robj, op);
	}

	shared_ptr<IceObject> ExprStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return assignment->runCode(top);
	}

	shared_ptr<IceObject> VariableDeclarationStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
		top->put(id->name, obj);
		return nullptr;
	}

	shared_ptr<IceObject> VariableAssignStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = assignment->runCode(top);
		top->replace(id->name, obj);
		return nullptr;
	}

	shared_ptr<IceObject> FunctionDeclarationStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = make_shared<IceFunctionObject>(argDecls, block);
		top->put(id->name, obj);
		return nullptr;
	}

	shared_ptr<IceObject> ClassDeclarationStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = make_shared<IceClassObject>(bases, block);
		top->put(id->name, obj);
		return nullptr;
	}

	shared_ptr<IceObject> BreakStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		top->setBreakStatus(true);
		return nullptr;
	}

	shared_ptr<IceObject> ContinueStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		top->setContinueStatus(true);
		return nullptr;
	}

	shared_ptr<IceObject> ReturnStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> returnValue = assignment->runCode(top);
		top->setReturnValue(returnValue);
		return nullptr;
	}

	shared_ptr<IceObject> UsingStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		runUsingStmt(name, top);
		return nullptr;
	}

	shared_ptr<IceObject> IfElseStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> cond = this->cond->runCode(top);
		shared_ptr<IceObject> returnValue = nullptr;
		if (cond->isTrue())
		{
			shared_ptr<Env> _top = make_shared<Env>(top);
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
			shared_ptr<Env> _top = make_shared<Env>(top);
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
		if (returnValue != nullptr)
		{
			top->setReturnValue(returnValue);
		}
		return returnValue;
	}

	shared_ptr<IceObject> WhileStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> cond = this->cond->runCode(top);
		shared_ptr<IceObject> returnValue = nullptr;
		shared_ptr<Env> _top = make_shared<Env>(top);

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

	shared_ptr<IceObject> DoWhileStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> cond = this->cond->runCode(top);
		shared_ptr<IceObject> returnValue = nullptr;
		shared_ptr<Env> _top = make_shared<Env>(top);

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

	shared_ptr<IceObject> ForStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> begin = this->begin->runCode(top);
		shared_ptr<IceObject> end = this->end->runCode(top);
		shared_ptr<IceObject> returnValue = nullptr;

		shared_ptr<Env> _top = make_shared<Env>(top);

		while (begin->binaryOperate(end, TOKEN::TCLT)->isTrue())
		{
			returnValue = block->runCode(_top);
			if (_top->getBreakStatus())
			{
				return returnValue;
			}
			if (_top->getContinueStatus())
			{
				_top->setContinueStatus(false);
				begin = begin->binaryOperate(make_shared<IceIntegerObject>(1), TOKEN::TADD);
				continue;
			}
			if (returnValue != nullptr)
			{
				top->setReturnValue(returnValue);
				return returnValue;
			}
			begin = begin->binaryOperate(make_shared<IceIntegerObject>(1), TOKEN::TADD);
		}
		return returnValue;
	}

	shared_ptr<IceObject> LambdaExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		return make_shared<IceFunctionObject>(argDecls, block);
	}

	shared_ptr<IceObject> NewExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceClassObject> class_obj = dynamic_pointer_cast<IceClassObject>(top->getObject(id->name));
		shared_ptr<IceInstanceObject> ins_obj = make_shared<IceInstanceObject>(top);

		for (auto &base : class_obj->bases)
		{
			dynamic_pointer_cast<IceClassObject>(top->getObject(base->name))->block->runCode(ins_obj->top);
			if (ins_obj->top->getObject(base->name)->type == IceObject::TYPE::FUNCTION)
			{
				shared_ptr<MethodCallExpr> call = make_shared<MethodCallExpr>(make_shared<IdentifierExpr>(base->name), arguments);
				call->runCode(ins_obj->top);
			}
		}

		class_obj->block->runCode(ins_obj->top);
		ins_obj->top->put("self", ins_obj);

		if (ins_obj->top->getObject(id->name)->type == IceObject::TYPE::FUNCTION)
		{
			shared_ptr<MethodCallExpr> call = make_shared<MethodCallExpr>(make_shared<IdentifierExpr>(id->name), arguments);
			call->runCode(ins_obj->top);
		}
		return ins_obj;
	}

	shared_ptr<IceObject> DotExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		if (normal_top == nullptr)
		{
			shared_ptr<IceObject> _obj = left->runCode(top, top);
			if (!IceObject::isInstance(_obj->type))
			{
				cout << "it doesn't support for '.'" << endl;
				exit(0);
			}

			shared_ptr<IceInstanceObject> obj = dynamic_pointer_cast<IceInstanceObject>(_obj);
			shared_ptr<IceObject> res = right->runCode(obj->top, top);
			return res;
		}

		shared_ptr<IceObject> _obj = left->runCode(top, normal_top);
		if (!IceObject::isInstance(_obj->type)) 
		{
			cout << "it doesn't support for '.'" << endl;
			exit(0);
		}

		shared_ptr<IceInstanceObject> obj = dynamic_pointer_cast<IceInstanceObject>(_obj);
		shared_ptr<IceObject> res = right->runCode(obj->top, normal_top);
		return res;
	}

	shared_ptr<IceObject> DotStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<Env> _top = top;
		for (auto &expression : expressions)
		{
			shared_ptr<IceObject> _obj = expression->runCode(_top);
			if (!IceObject::isInstance(_obj->type))
			{
				cout << "it doesn't '.' operator" << endl;
				exit(0);
			}
			
			shared_ptr<IceInstanceObject> obj = dynamic_pointer_cast<IceInstanceObject>(_obj);
			_top = obj->top;
		}
		to_run->runCode(_top, top);
		return nullptr;
	}

	shared_ptr<IceObject> EnumExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceInstanceObject> obj = make_shared<IceInstanceObject>(top);;

		long i = 0;
		for (auto &enumerator : enumerators)
		{
			obj->top->put(enumerator->name, make_shared<IceIntegerObject>(i++));
		}

		return obj;
	}

	shared_ptr<IceObject> MatchExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> obj = expression->runCode(top);
		for (size_t i = 0; i < mat_expressions.size(); i++)
		{
			if (mat_expressions[i]->runCode(top)->binaryOperate(obj, TOKEN::TCEQ)->isTrue())
			{
				return ret_expressions[i]->runCode(top);
			}
		}
		return else_expression->runCode(top);
	}

	shared_ptr<IceObject> ListExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceListObject> obj = make_shared<IceListObject>();
		for (auto &expression : expressions)
		{
			obj->objects.push_back(expression->runCode(top));
		}
		return obj;
	}

	shared_ptr<IceObject> IndexExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type != IceObject::TYPE::LIST && _obj->type != IceObject::TYPE::STRING && _obj->type != IceObject::TYPE::DICT)
		{
			cout << "it doesn't support for []" << endl;
			exit(0);
		}

		if (_obj->type == IceObject::TYPE::LIST) 
		{
			shared_ptr<IceListObject> obj = dynamic_pointer_cast<IceListObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
		else if (_obj->type == IceObject::TYPE::STRING)
		{
			shared_ptr<IceStringObject> obj = dynamic_pointer_cast<IceStringObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
		else
		{
			shared_ptr<IceDictObject> obj = dynamic_pointer_cast<IceDictObject>(_obj);
			return obj->getByIndex(index->runCode((normal_top == nullptr) ? (top) : (normal_top)));
		}
	}

	shared_ptr<IceObject> IndexStmt::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		shared_ptr<IceObject> _obj = expression->runCode(top);
		if (_obj->type != IceObject::TYPE::LIST && _obj->type != IceObject::TYPE::DICT)
		{
			cout << "it doesn't support for []" << endl;
			exit(0);
		}

		if (_obj->type == IceObject::TYPE::LIST)
		{
			shared_ptr<IceListObject> obj = dynamic_pointer_cast<IceListObject>(_obj);
			shared_ptr<IceObject> index = this->index->runCode((normal_top == nullptr) ? (top) : (normal_top));
			shared_ptr<IceObject> assignment = this->assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
			obj->setByIndex(index, assignment);
		}
		else
		{
			shared_ptr<IceDictObject> obj = dynamic_pointer_cast<IceDictObject>(_obj);
			shared_ptr<IceObject> index = this->index->runCode((normal_top == nullptr) ? (top) : (normal_top));
			shared_ptr<IceObject> assignment = this->assignment->runCode((normal_top == nullptr) ? (top) : (normal_top));
			obj->setByIndex(index, assignment);
		}
		return nullptr;
	}

	shared_ptr<IceObject> DictExpr::runCode(shared_ptr<Env> &top, shared_ptr<Env> normal_top)
	{
		if (keys.size() != values.size())
		{
			cout << "one key, one value" << endl;
			exit(0);
		}
		shared_ptr<IceDictObject> obj = make_shared<IceDictObject>();
		for (size_t i = 0; i < keys.size(); i++)
		{
			obj->setByIndex(keys[i]->runCode(top), values[i]->runCode(top));
		}
		return obj;
	}

	// Built_in functions implement

	void Env::genBuildInFunctions()
	{
		put("input", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size())
			{
				cout << "input() need no arguments" << endl;
				exit(0);
			}
			string input;
			getline(cin, input);
			return dynamic_pointer_cast<IceObject>(make_shared<IceStringObject>(input));
		}));

		put("print", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "print() need 1 argument but get others" << endl;
				exit(0);
			}
			cout << objects[0]->toStr();
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		put("println", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "println() need 1 argument but get others" << endl;
				exit(0);
			}
			cout << objects[0]->toStr() << endl;
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		put("str", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "str() need 1 argument but get others" << endl;
				exit(0);
			}
			return dynamic_pointer_cast<IceObject>(make_shared<IceStringObject>(objects[0]->toStr()));
		}));

		put("exit", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size())
			{
				if (objects.size() == 1)
				{
					if (objects[0]->type == IceObject::TYPE::INT)
					{
						exit(dynamic_pointer_cast<IceIntegerObject>(objects[0])->value);
					}
				}
				else
				{
					cout << "exit() need no arguments" << endl;
					exit(0);
				}
			}
			exit(0);
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));


		put("len", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "len() need 1 argument but get others" << endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING && objects[0]->type != IceObject::TYPE::LIST)
			{
				cout << "len() need string object or list object but get other types" << endl;
				exit(0);
			}
			if (objects[0]->type == IceObject::TYPE::STRING)
			{
				return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(dynamic_pointer_cast<IceStringObject>(objects[0])->value.length()));
			}
			else
			{
				return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(dynamic_pointer_cast<IceListObject>(objects[0])->objects.size()));
			}
		}));

		put("int", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "int() need 1 arguments but get others" << endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING)
			{
				cout << "int() need string object" << endl;
				exit(0);
			}
			return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(atoi(dynamic_pointer_cast<IceStringObject>(objects[0])->value.c_str())));
		}));

		put("float", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "float() need 1 arguments but get others" << endl;
				exit(0);
			}
			else if (objects[0]->type != IceObject::TYPE::STRING)
			{
				cout << "float() need string object" << endl;
				exit(0);
			}
			return dynamic_pointer_cast<IceObject>(make_shared<IceDoubleObject>(atof(dynamic_pointer_cast<IceStringObject>(objects[0])->value.c_str())));
		}));

		put("time", make_shared<IceBuiltInFunctionObject>([](Objects objects) 
		{
			if (objects.size())
			{
				cout << "time() need no arguments" << endl;
				exit(0);
			}
			return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(time(nullptr)));
		}));
	}
}
