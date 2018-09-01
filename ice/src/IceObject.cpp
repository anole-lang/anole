#include "IceObject.h"

using ::std::endl;
using ::std::hash;
using ::std::reverse;

using TOKEN = Ice::Token::TOKEN;

namespace Ice
{
	// General method definitions
	bool IceObject::isInstance()
	{
		return type == TYPE::INSTANCE || type == TYPE::LIST || type == TYPE::STRING || type == TYPE::DICT;
	}

	bool IceObject::isTraversable()
	{
		return type == TYPE::LIST || type == TYPE::STRING || type == TYPE::DICT;
	}


	// Definitions for IceFunctionObject
	IceFunctionObject::IceFunctionObject(const VariableList &argDecls, shared_ptr<BlockExpr> block) : argDecls(argDecls), block(block)
	{
		type = TYPE::FUNCTION;
	}
	

	// Definitions for IceBuiltInFunctionObject
	IceBuiltInFunctionObject::IceBuiltInFunctionObject(function<shared_ptr<IceObject>(Objects)> func) : func(func)
	{
		type = TYPE::BUILT_IN_FUNCTION;
	}


	// Definitions for IceClassObject
	IceClassObject::IceClassObject(const IdentifierList &bases, shared_ptr<BlockExpr> block) : bases(bases), block(block)
	{
		type = TYPE::CLASS;
	}


	// Definitions for IceInstancesObject
	IceInstanceObject::IceInstanceObject()
	{
		type = TYPE::INSTANCE;
		top = make_shared<Env>(nullptr);
	}

	IceInstanceObject::IceInstanceObject(shared_ptr<Env> &top)
	{
		type = TYPE::INSTANCE;
		this->top = make_shared<Env>(top);
	}

	shared_ptr<IceObject> IceInstanceObject::binaryOperate(shared_ptr<IceObject> obj, TOKEN op)
	{
		if (obj->type == TYPE::NONE && op == TOKEN::TCNE)
		{
			return make_shared<IceBooleanObject>(true);
		}
		else if (obj->type == TYPE::NONE && op == TOKEN::TCEQ)
		{
			return make_shared<IceBooleanObject>(false);
		}
		else
		{
			return make_shared<IceBooleanObject>(false);
		}
	}


	// Definitions for IceNoneObject
	IceNoneObject::IceNoneObject()
	{
		type = TYPE::NONE;
	}


	// Definitions for IceIntegerObejct
	IceIntegerObject::IceIntegerObject(long value) : value(value)
	{
		type = TYPE::INT;
	}

	shared_ptr<IceObject> IceIntegerObject::unaryOperate(TOKEN op)
	{
		switch (op)
		{
		case TOKEN::TSUB:
			return make_shared<IceIntegerObject>(-value);
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}

	shared_ptr<IceObject> IceIntegerObject::binaryOperate(shared_ptr<IceObject> obj, TOKEN op)
	{
		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceIntegerObject>(value + dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceIntegerObject>(value - dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceIntegerObject>(value * dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceIntegerObject>(value / dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceIntegerObject>(value % dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceDoubleObject>(value + dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceDoubleObject>(value - dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceDoubleObject>(value * dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceDoubleObject>(value / dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceDoubleObject>(value % (long)dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}


	// Definitions for IceDoubleObject
	IceDoubleObject::IceDoubleObject(double value) : value(value)
	{
		type = TYPE::DOUBLE;
	}

	shared_ptr<IceObject> IceDoubleObject::unaryOperate(TOKEN op)
	{
		switch (op)
		{
		case TOKEN::TSUB:
			return make_shared<IceDoubleObject>(-value);
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}

	shared_ptr<IceObject> IceDoubleObject::binaryOperate(shared_ptr<IceObject> obj, TOKEN op)
	{

		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceDoubleObject>(value + dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceDoubleObject>(value - dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceDoubleObject>(value * dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceDoubleObject>(value / dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceDoubleObject>((long)value % dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceDoubleObject>(value + dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceDoubleObject>(value - dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceDoubleObject>(value * dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceDoubleObject>(value / dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceDoubleObject>((long)value % (long)dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}


	// Definitions for IceBooleanObject
	IceBooleanObject::IceBooleanObject(bool value) : value(value)
	{
		type = TYPE::BOOLEAN;
	}

	void IceBooleanObject::show()
	{
		if (value) cout << "true";
		else cout << "false";
	}

	shared_ptr<IceObject> IceBooleanObject::unaryOperate(TOKEN op)
	{
		switch (op)
		{
		case TOKEN::TSUB:
			return make_shared<IceIntegerObject>(-value);
		case TOKEN::TNOT:
			return make_shared<IceBooleanObject>(!value);
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}

	shared_ptr<IceObject> IceBooleanObject::binaryOperate(shared_ptr<IceObject> obj, TOKEN op)
	{
		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceIntegerObject>(value + dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceIntegerObject>(value - dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceIntegerObject>(value * dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceIntegerObject>(value / dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceIntegerObject>(value % dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceDoubleObject>(value + dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TSUB:
				return make_shared<IceDoubleObject>(value - dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMUL:
				return make_shared<IceDoubleObject>(value * dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TDIV:
				return make_shared<IceDoubleObject>(value / dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TMOD:
				return make_shared<IceDoubleObject>(value % (long)dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLT:
				return make_shared<IceBooleanObject>(value < dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCLE:
				return make_shared<IceBooleanObject>(value <= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGT:
				return make_shared<IceBooleanObject>(value > dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case TOKEN::TCGE:
				return make_shared<IceBooleanObject>(value >= dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::BOOLEAN:
			switch (op)
			{
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			case TOKEN::TAND:
				return make_shared<IceBooleanObject>(value && dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			case TOKEN::TOR:
				return make_shared<IceBooleanObject>(value || dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			default:
				break;
			}
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}


	// Definitions for IceStringObject
	IceStringObject::IceStringObject(string value) : value(value)
	{
		genBuiltInMethods();

		type = TYPE::STRING;

		raw_value = "";
		const char *p = value.c_str();
		while (*p)
		{
			switch (*p)
			{
			case '\n':
				raw_value += "\\n";
				break;
			case '\\':
				raw_value += "\\\\";
				break;
			case '\"':
				raw_value += "\\\"";
				break;
			case '\a':
				raw_value += "\\a";
				break;
			case '\b':
				raw_value += "\\b";
				break;
			case '\0':
				raw_value += "\\0";
				break;
			case '\t':
				raw_value += "\\t";
				break;
			case '\r':
				raw_value += "\\r";
				break;
			case '\f':
				raw_value += "\\f";
				break;
			default:
				raw_value += *p;
				break;
			}
			++p;
		}
	}

	shared_ptr<IceObject> IceStringObject::unaryOperate(TOKEN op)
	{
		string dup = value;
		switch (op)
		{
		case TOKEN::TSUB:
			reverse(dup.begin(), dup.end());
			return make_shared<IceStringObject>(dup);
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}

	shared_ptr<IceObject> IceStringObject::binaryOperate(shared_ptr<IceObject> obj, TOKEN op)
	{
		string dup = value;
		switch (obj->type)
		{
		case TYPE::STRING:
			switch (op)
			{
			case TOKEN::TADD:
				return make_shared<IceStringObject>(value + dynamic_pointer_cast<IceStringObject>(obj)->value);
			case TOKEN::TCEQ:
				return make_shared<IceBooleanObject>(value == dynamic_pointer_cast<IceStringObject>(obj)->value);
			case TOKEN::TCNE:
				return make_shared<IceBooleanObject>(value != dynamic_pointer_cast<IceStringObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::INT:
			switch (op)
			{
			case TOKEN::TMUL:
				for (int i = 1; i < dynamic_pointer_cast<IceIntegerObject>(obj)->value; i++) dup += value;
				return make_shared<IceStringObject>(dup);
			case TOKEN::TCEQ:
				return make_shared<IceIntegerObject>(0);
			case TOKEN::TCNE:
				return make_shared<IceIntegerObject>(1);
			default:
				break;
			}
			break;
		default:
			break;
		}
		cout << "doesn't support this operator" << endl;
		exit(0);
		return nullptr;
	}

	shared_ptr<IceObject> IceStringObject::getByIndex(shared_ptr<IceObject> _index)
	{
		if (_index->type != TYPE::INT)
		{
			cout << "index type should be integer" << endl;
			exit(0);
		}
		shared_ptr<IceIntegerObject> index = dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)value.size())
		{
			cout << "index out of range" << endl;
			exit(0);
		}
		string str;
		str += value[index->value];
		shared_ptr<IceStringObject> obj = make_shared<IceStringObject>(str);
		obj->top->put("self", obj);
		return obj;
	}

	void IceStringObject::genBuiltInMethods()
	{
		top->put("isalpha", make_shared<IceBuiltInFunctionObject>([&](Objects objects)
		{
			if (objects.size())
			{
				cout << "method isalpha() need no arguments" << endl;
				exit(0);
			}
			return dynamic_pointer_cast<IceObject>(make_shared<IceBooleanObject>(value.size() == 1 ? (isalpha(value[0])) : false));
		}));

		top->put("size", make_shared<IceBuiltInFunctionObject>([&](Objects objects)
		{
			if (objects.size())
			{
				cout << "method size() need no arguments" << endl;
				exit(0);
			}

			return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(value.size()));
		}));
	}


	// Definitions for IceListObject
	IceListObject::IceListObject()
	{
		type = TYPE::LIST;
		genBuiltInMethods();
	}

	IceListObject::IceListObject(Objects objects) : objects(objects)
	{
		type = TYPE::LIST;
		genBuiltInMethods();
	}

	void IceListObject::show()
	{
		cout << "[";
		for (size_t i = 0; i < objects.size(); i++)
		{
			if (i) cout << ", ";
			objects[i]->show();
		}
		cout << "]";
	}

	shared_ptr<IceObject> IceListObject::unaryOperate(TOKEN op)
	{
		shared_ptr<IceListObject> obj = nullptr;
		switch (op)
		{
		case TOKEN::TSUB:
			obj = make_shared<IceListObject>();
			for (int i = (int)objects.size() - 1; i >= 0; i--)
			{
				obj->objects.push_back(objects[i]);
			}
			break;
		default:
			break;
		}
		return obj;
	}

	shared_ptr<IceObject> IceListObject::binaryOperate(shared_ptr<IceObject> _obj, TOKEN op)
	{
		if (_obj->type != TYPE::LIST)
		{
			cout << "doesn't support this operator" << endl;
			exit(0);
		}
		shared_ptr<IceListObject> obj = dynamic_pointer_cast<IceListObject>(_obj);
		shared_ptr<IceListObject> res_obj = make_shared<IceListObject>(objects);
		switch (op)
		{
		case TOKEN::TADD:
			for (auto &object : obj->objects)
			{
				res_obj->objects.push_back(object);
			}
			break;
		default:
			break;
		}
		return res_obj;
	}

	shared_ptr<IceObject> IceListObject::getByIndex(shared_ptr<IceObject> _index)
	{
		if (_index->type != TYPE::INT)
		{
			cout << "index type should be integer" << endl;
			exit(0);
		}
		shared_ptr<IceIntegerObject> index = dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)objects.size())
		{
			cout << "index out of range" << endl;
			exit(0);
		}
		return objects[index->value];
	}

	void IceListObject::setByIndex(shared_ptr<IceObject> _index, shared_ptr<IceObject> assignment)
	{
		if (_index->type != TYPE::INT)
		{
			cout << "index type should be integer" << endl;
			exit(0);
		}
		shared_ptr<IceIntegerObject> index = dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)objects.size())
		{
			cout << "index out of range" << endl;
			exit(0);
		}
		objects[index->value] = assignment;
	}

	void IceListObject::genBuiltInMethods()
	{
		top->put("size", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method size() need no arguments" << endl;
				exit(0);
			}

			return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(objects.size()));
		}));

		top->put("empty", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method empty() need no arguments" << endl;
				exit(0);
			}

			return dynamic_pointer_cast<IceObject>(make_shared<IceBooleanObject>(objects.empty()));
		}));

		top->put("push_back", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size() != 1)
			{
				cout << "method push_back() need 1 argument but get others" << endl;
				exit(0);
			}

			objects.push_back(_objects[0]);
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		top->put("pop_back", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method pop_back() need no arguments" << endl;
				exit(0);
			}

			objects.pop_back();
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		top->put("front", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method front() need no arguments" << endl;
				exit(0);
			}

			return objects.front();
		}));

		top->put("back", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method back() need no arguments" << endl;
				exit(0);
			}

			return objects.back();
		}));

		top->put("clear", make_shared<IceBuiltInFunctionObject>([&](Objects _objects)
		{
			if (_objects.size())
			{
				cout << "method clear() need no arguments" << endl;
				exit(0);
			}

			objects.clear();
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));
	}


	// Definitions for IceDictObject
	IceDictObject::IceDictObject()
	{
		type = TYPE::DICT;
		genBuiltInMethods();
	}

	size_t KeyObject::hashValue() const
	{
		switch (obj->type)
		{
		case IceObject::TYPE::INT:
			return hash<long>{}(dynamic_pointer_cast<IceIntegerObject>(obj)->value);
		case IceObject::TYPE::DOUBLE:
			return hash<double>{}(dynamic_pointer_cast<IceDoubleObject>(obj)->value);
		case IceObject::TYPE::BOOLEAN:
			return hash<bool>{}(dynamic_pointer_cast<IceBooleanObject>(obj)->value);
		case IceObject::TYPE::STRING:
			return hash<string>{}(dynamic_pointer_cast<IceStringObject>(obj)->value);
		default:
			break;
		}
		cout << "key for dict should be int, double, boolean or string" << endl;
		exit(0);
		return 0;
	}

	void IceDictObject::show()
	{
		cout << "{";
		for (auto iter = objects_map.begin(); iter != objects_map.end(); iter++)
		{
			if (iter != objects_map.begin())
			{
				cout << ", ";
			}
			iter->first.obj->show();
			cout << ": ";
			iter->second->show();
		}
		cout << "}";
	}

	shared_ptr<IceObject> IceDictObject::getByIndex(shared_ptr<IceObject> _key)
	{
		KeyObject key(_key);
		if (objects_map.find(key) != objects_map.end())
		{
			return objects_map[key];
		}
		else
		{
			return make_shared<IceNoneObject>();
		}
	}

	void IceDictObject::setByIndex(shared_ptr<IceObject> key, shared_ptr<IceObject> value)
	{
		objects_map[KeyObject(key)] = value;
	}

	void IceDictObject::genBuiltInMethods()
	{
		top->put("at", make_shared<IceBuiltInFunctionObject>([&](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "method at() need one argument" << endl;
				exit(0);
			}

			return getByIndex(objects[0]);
		}));

		top->put("empty", make_shared<IceBuiltInFunctionObject>([&](Objects objects)
		{
			if (objects.size())
			{
				cout << "method empty() need no arguments" << endl;
				exit(0);
			}

			return dynamic_pointer_cast<IceObject>(make_shared<IceBooleanObject>(objects_map.empty()));
		}));

		top->put("size", make_shared<IceBuiltInFunctionObject>([&](Objects objects)
		{
			if (objects.size())
			{
				cout << "method size() need no arguments" << endl;
				exit(0);
			}

			return dynamic_pointer_cast<IceObject>(make_shared<IceIntegerObject>(objects_map.size()));
		}));

		top->put("clear", make_shared<IceBuiltInFunctionObject>([&](Objects objects) 
		{
			if (objects.size())
			{
				cout << "method clear() need no arguments" << endl;
				exit(0);
			}

			objects_map.clear();
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		top->put("insert", make_shared<IceBuiltInFunctionObject>([&](Objects objects)
		{
			if (objects.size() != 2)
			{
				cout << "method insert() need 2 arguments" << endl;
				exit(0);
			}

			setByIndex(objects[0], objects[1]);
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));

		top->put("erase", make_shared<IceBuiltInFunctionObject>([&](Objects objects) 
		{
			if (objects.size() != 1)
			{
				cout << "method erase() need 1 argument" << endl;
				exit(0);
			}

			objects_map.erase(KeyObject(objects[0]));
			return dynamic_pointer_cast<IceObject>(make_shared<IceNoneObject>());
		}));
	}
}

