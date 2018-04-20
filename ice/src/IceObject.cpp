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

IceObject *IceIntegerObject::binaryOperate(IceObject *obj, Token::TOKEN op)
{
    switch (obj->type)
    {
    case TYPE::INT:
        switch (op)
        {
        case Token::TOKEN::TADD:
            return new IceIntegerObject(value + dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TSUB:
            return new IceIntegerObject(value - dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TMUL:
            return new IceIntegerObject(value * dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TDIV:
            return new IceIntegerObject(value / dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TMOD:
            return new IceIntegerObject(value % dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCEQ:
            return new IceIntegerObject(value == dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCNE:
            return new IceIntegerObject(value != dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCLT:
            return new IceIntegerObject(value < dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCLE:
            return new IceIntegerObject(value <= dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCGT:
            return new IceIntegerObject(value > dynamic_cast<IceIntegerObject *>(obj)->value);
        case Token::TOKEN::TCGE:
            return new IceIntegerObject(value >= dynamic_cast<IceIntegerObject *>(obj)->value);
        default:
            break;
        }
        break;
    default:
        break;
    }
    return nullptr;
}
}

