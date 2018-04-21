#include "IceObject.h"

namespace Ice
{
	IceIntegerObject::IceIntegerObject(long value) : value(value)
	{
		type = TYPE::INT;
	}

	void IceIntegerObject::show()
	{
		std::cout << value << std::endl;
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
				return std::make_shared<IceIntegerObject>(value == std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCNE:
				return std::make_shared<IceIntegerObject>(value != std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLT:
				return std::make_shared<IceIntegerObject>(value < std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCLE:
				return std::make_shared<IceIntegerObject>(value <= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGT:
				return std::make_shared<IceIntegerObject>(value > std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			case Token::TOKEN::TCGE:
				return std::make_shared<IceIntegerObject>(value >= std::dynamic_pointer_cast<IceIntegerObject>(obj)->value);
			default:
				break;
			}
			break;
		default:
			break;
		}
		return nullptr;
	}

	IceDoubleObject::IceDoubleObject(double value)
	{
		type = TYPE::DOUBLE;
	}

	void IceDoubleObject::show()
	{
		std::cout << value << std::endl;
	}

	std::shared_ptr<IceObject> IceDoubleObject::binaryOperate(std::shared_ptr<IceObject> obj, Token::TOKEN op)
	{
		return nullptr;
	}
}

