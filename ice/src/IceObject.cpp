#include "IceObject.h"

namespace Ice
{
	IceFunctionObject::IceFunctionObject(const VariableList &argDecls, std::shared_ptr<BlockExpr> block) : argDecls(argDecls), block(block)
	{
		type = TYPE::FUNCTION;
	}

	IceClassObject::IceClassObject(const IdentifierList &bases, std::shared_ptr<BlockExpr> block) : bases(bases), block(block)
	{
		type = TYPE::CLASS;
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
		case Ice::Token::TOKEN::TSUB:
			return std::make_shared<IceIntegerObject>(-value);
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

	void IceListObject::show()
	{
		std::cout << "[";
		for (auto &obj : objects)
		{
			obj->show();
			std::cout << ", ";
		}
		std::cout << "]";
	}

	std::shared_ptr<IceObject> IceListObject::getByIndex(std::shared_ptr<IceObject> _obj)
	{
		if (_obj->type != TYPE::INT)
		{
			std::cout << "index type should be int" << std::endl;
			exit(0);
		}
		std::shared_ptr<IceIntegerObject> obj = std::dynamic_pointer_cast<IceIntegerObject>(_obj);

		if (obj->value >= (int)objects.size())
		{
			std::cout << "index out of range" << std::endl;
			exit(0);
		}
		return objects[obj->value];
	}
}

