#ifndef __ICE_OBJECT_H__
#define __ICE_OBJECT_H__

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include "Token.h"

namespace Ice
{
	class BlockExpr;
	class IdentifierExpr;
	typedef std::vector<std::shared_ptr<IdentifierExpr>> VariableList;

	class IceObject
	{
	public:
		enum class TYPE
		{
			INT,
			DOUBLE,
			BOOLEAN,
			STRING,
			FUNCTION
		} type;
		virtual ~IceObject() {}
		virtual void show() = 0;
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op) = 0;
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN) = 0;
		virtual bool isTrue() = 0;
		virtual std::string toStr() = 0;
	};

	class IceFunctionObject : public IceObject
	{
	public:
		VariableList arguments;
		std::shared_ptr<BlockExpr> block;
		IceFunctionObject(const VariableList &, std::shared_ptr<BlockExpr>);

		virtual ~IceFunctionObject() {}
		virtual void show() {}
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op) { return nullptr; };
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN) { return nullptr; }
		virtual bool isTrue() { return true; }
		virtual std::string toStr() { return "function at " + (int)this; }
	};

	class IceIntegerObject : public IceObject
	{
	public:
		long value;

		IceIntegerObject(long value);
		virtual ~IceIntegerObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue();
		virtual std::string toStr() { return std::to_string(value); }
	};

	class IceDoubleObject : public IceObject
	{
	public:
		double value;

		IceDoubleObject(double value);
		virtual ~IceDoubleObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue();
		virtual std::string toStr() { return std::to_string(value); }
	};

	class IceBooleanObject : public IceObject
	{
	public:
		bool value;

		IceBooleanObject(bool value);
		virtual ~IceBooleanObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue();
		virtual std::string toStr() { return std::to_string(value); }
	};

	class IceStringObject : public IceObject
	{
	public:
		std::string value;

		IceStringObject(std::string value);
		virtual ~IceStringObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue();
		virtual std::string toStr() { return value; }
	};
}

#endif // __ICE_OBJECT_H__