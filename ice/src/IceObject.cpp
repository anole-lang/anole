#include "IceObject.h"

namespace Ice
{
	IceFunctionObject::IceFunctionObject(const VariableList &argDecls, std::shared_ptr<BlockExpr> block) : argDecls(argDecls), block(block)
	{
		type = TYPE::FUNCTION;
	}

	IceBuiltInFunctionObject::IceBuiltInFunctionObject(std::function<std::shared_ptr<IceObject>(Objects)> func) : func(func)
	{
		type = TYPE::BUILT_IN_FUNCTION;
	}

	IceClassObject::IceClassObject(const IdentifierList &bases, std::shared_ptr<BlockExpr> block) : bases(bases), block(block)
	{
		type = TYPE::CLASS;
	}

	IceInstanceObject::IceInstanceObject()
	{
		type = TYPE::INSTANCE;
		top = std::make_shared<Env>(nullptr);
	}

	IceInstanceObject::IceInstanceObject(std::shared_ptr<Env> &top)
	{
		type = TYPE::INSTANCE;
		this->top = std::make_shared<Env>(top);
	}

	IceNoneObject::IceNoneObject()
	{
		type = TYPE::NONE;
	}

	IceIntegerObject::IceIntegerObject(long value) : value(value)
	{
		type = TYPE::INT;
	}

	IceDoubleObject::IceDoubleObject(double value) : value(value)
	{
		type = TYPE::DOUBLE;
	}

	IceBooleanObject::IceBooleanObject(bool value) : value(value)
	{
		type = TYPE::BOOLEAN;
	}

	IceStringObject::IceStringObject(std::string value) : value(value)
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
			p++;
		}
	}

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

	std::shared_ptr<IceObject> IceIntegerObject::unaryOperate(Token::TOKEN op)
	{
		switch (op)
		{
		case Ice::Token::TOKEN::TSUB:
			return std::make_shared<IceIntegerObject>(-value);
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceIntegerObject::binaryOperate(std::shared_ptr<IceObject> obj, Token::TOKEN op)
	{
		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceIntegerObject>(value + std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceIntegerObject>(value - std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceIntegerObject>(value * std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceIntegerObject>(value / std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceIntegerObject>(value % std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceDoubleObject>(value + std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceDoubleObject>(value - std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceDoubleObject>(value * std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceDoubleObject>(value / std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceDoubleObject>(value % (long)std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceDoubleObject::unaryOperate(Token::TOKEN op)
	{
		switch (op)
		{
		case Ice::Token::TOKEN::TSUB:
			return std::make_shared<IceDoubleObject>(-value);
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceDoubleObject::binaryOperate(std::shared_ptr<IceObject> obj, Token::TOKEN op)
	{

		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceDoubleObject>(value + std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceDoubleObject>(value - std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceDoubleObject>(value * std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceDoubleObject>(value / std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceDoubleObject>((long)value % std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceDoubleObject>(value + std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceDoubleObject>(value - std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceDoubleObject>(value * std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceDoubleObject>(value / std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceDoubleObject>((long)value % (long)std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	void IceBooleanObject::show()
	{
		if (value) std::cout << "true";
		else std::cout << "false";
	}

	std::shared_ptr<IceObject> IceBooleanObject::unaryOperate(Token::TOKEN op)
	{
		switch (op)
		{
		case Token::TOKEN::TSUB:
			return std::make_shared<IceIntegerObject>(-value);
		case Token::TOKEN::TNOT:
			return std::make_shared<IceBooleanObject>(!value);
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceBooleanObject::binaryOperate(std::shared_ptr<IceObject> obj, Token::TOKEN op)
	{
		switch (obj->type)
		{
		case TYPE::INT:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceIntegerObject>(value + std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceIntegerObject>(value - std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceIntegerObject>(value * std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceIntegerObject>(value / std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceIntegerObject>(value % std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::DOUBLE:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceDoubleObject>(value + std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TSUB:
				return std::make_shared<IceDoubleObject>(value - std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMUL:
				return std::make_shared<IceDoubleObject>(value * std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TDIV:
				return std::make_shared<IceDoubleObject>(value / std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TMOD:
				return std::make_shared<IceDoubleObject>(value % (long)std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceBooleanObject>(value < std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceBooleanObject>(value <= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceBooleanObject>(value > std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceBooleanObject>(value >= std::dynamic_pointer_cast<IceDoubleObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::BOOLEAN:
			switch (op)
			{
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			case Token::TOKEN::TAND:
				return std::make_shared<IceBooleanObject>(value && std::dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			case Token::TOKEN::TOR:
				return std::make_shared<IceBooleanObject>(value || std::dynamic_pointer_cast<IceBooleanObject>(obj)->value);
			default:
				break;
			}
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceStringObject::unaryOperate(Token::TOKEN op)
	{
		std::string dup = value;
		switch (op)
		{
		case Ice::Token::TOKEN::TSUB:
			std::reverse(dup.begin(), dup.end());
			return std::make_shared<IceStringObject>(dup);
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceStringObject::binaryOperate(std::shared_ptr<IceObject> obj, Token::TOKEN op)
	{
		std::string dup = value;
		switch (obj->type)
		{
		case TYPE::STRING:
			switch (op)
			{
			case Token::TOKEN::TADD:
				return std::make_shared<IceStringObject>(value + std::dynamic_pointer_cast<IceStringObject>(obj)->value);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceBooleanObject>(value == std::dynamic_pointer_cast<IceStringObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceBooleanObject>(value != std::dynamic_pointer_cast<IceStringObject>(obj)->value);
			default:
				break;
			}
			break;
		case TYPE::INT:
			switch (op)
			{
			case Token::TOKEN::TMUL:
				for (int i = 1; i < std::dynamic_pointer_cast<IceIntegerObject>(obj)->value; i++) dup += value;
				return std::make_shared<IceStringObject>(dup);
			case Token::TOKEN::TCEQ:
				return std::make_shared<IceIntegerObject>(0);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceIntegerObject>(1);
			default:
				break;
			}
			break;
		default:
			break;
		}
		std::cout << "doesn't support this operator" << std::endl;
		exit(0);
		return nullptr;
	}

	std::shared_ptr<IceObject> IceStringObject::getByIndex(std::shared_ptr<IceObject> _index)
	{
		if (_index->type != TYPE::INT)
		{
			std::cout << "index type should be integer" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceIntegerObject> index = std::dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)value.size())
		{
			std::cout << "index out of range" << std::endl;
			exit(0);
		}
		std::string str;
		str += value[index->value];
		std::shared_ptr<IceStringObject> obj = std::make_shared<IceStringObject>(str);
		obj->top->put("self", obj);
		return obj;
	}

	void IceStringObject::genBuiltInMethods()
	{
		top->put("isalpha", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method isalpha() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceStringObject> obj = std::dynamic_pointer_cast<IceStringObject>(top->getObject("self"));
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceBooleanObject>(obj->value.size() == 1 ? (isalpha(obj->value[0])) : false));
		}));
	}

	void IceListObject::show()
	{
		std::cout << "[";
		for (size_t i = 0; i < objects.size(); i++)
		{
			if (i) std::cout << ", ";
			objects[i]->show();
		}
		std::cout << "]";
	}

	std::shared_ptr<IceObject> IceListObject::unaryOperate(Token::TOKEN op)
	{
		std::shared_ptr<IceListObject> obj = nullptr;
		switch (op)
		{
		case Token::TOKEN::TSUB:
			obj = std::make_shared<IceListObject>();
			for (int i = (int)objects.size() - 1; i >= 0; i--)
				obj->objects.push_back(objects[i]);
			break;
		default:
			break;
		}
		return obj;
	}

	std::shared_ptr<IceObject> IceListObject::binaryOperate(std::shared_ptr<IceObject> _obj, Token::TOKEN op)
	{
		if (_obj->type != TYPE::LIST)
		{
			std::cout << "doesn't support this operator" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(_obj);
		std::shared_ptr<IceListObject> res_obj = std::make_shared<IceListObject>(objects);
		switch (op)
		{
		case Token::TOKEN::TADD:
			for (auto &object : obj->objects)
				res_obj->objects.push_back(object);
			break;
		default:
			break;
		}
		return res_obj;
	}

	std::shared_ptr<IceObject> IceListObject::getByIndex(std::shared_ptr<IceObject> _index)
	{
		if (_index->type != TYPE::INT)
		{
			std::cout << "index type should be integer" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceIntegerObject> index = std::dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)objects.size())
		{
			std::cout << "index out of range" << std::endl;
			exit(0);
		}
		return objects[index->value];
	}

	void IceListObject::setByIndex(std::shared_ptr<IceObject> _index, std::shared_ptr<IceObject> assignment)
	{
		if (_index->type != TYPE::INT)
		{
			std::cout << "index type should be integer" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceIntegerObject> index = std::dynamic_pointer_cast<IceIntegerObject>(_index);

		if (index->value >= (int)objects.size())
		{
			std::cout << "index out of range" << std::endl;
			exit(0);
		}
		objects[index->value] = assignment;
	}

	void IceListObject::genBuiltInMethods()
	{
		top->put("size", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method size() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceIntegerObject>(obj->objects.size()));
		}));

		top->put("empty", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method empty() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceBooleanObject>(obj->objects.empty()));
		}));

		top->put("push_back", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size() != 1)
			{
				std::cout << "method push_back() need 1 argument but get others" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			obj->objects.push_back(objects[0]);
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceNoneObject>());
		}));

		top->put("pop_back", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method pop_back() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			obj->objects.pop_back();
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceNoneObject>());
		}));

		top->put("front", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method front() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			return obj->objects.front();
		}));

		top->put("back", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method back() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			return obj->objects.back();
		}));

		top->put("clear", std::make_shared<IceBuiltInFunctionObject>([&](Objects objects) {
			if (objects.size())
			{
				std::cout << "method clear() need no arguments" << std::endl;
				exit(0);
			}

			std::shared_ptr<IceListObject> obj = std::dynamic_pointer_cast<IceListObject>(top->getObject("self"));
			obj->objects.clear();
			return std::dynamic_pointer_cast<IceObject>(std::make_shared<IceNoneObject>());
		}));
	}
}

