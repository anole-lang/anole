#ifndef __ICE_OBJECT_H__
#define __ICE_OBJECT_H__

#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include "Token.h"

namespace Ice
{
	class IceObject
	{
	public:
		enum class TYPE
		{
			INT,
			DOUBLE,
			STRING
		} type;
		virtual ~IceObject() {}
		virtual void show() = 0;
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN op) = 0;
		virtual bool isTrue() = 0;
	};

	class IceIntegerObject : public IceObject
	{
	private:
		long value;

	public:
		IceIntegerObject(long value);
		virtual ~IceIntegerObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN op);
		virtual bool isTrue();
	};

	class IceDoubleObject : public IceObject
	{
	private:
		double value;

	public:
		IceDoubleObject(double value);
		virtual ~IceDoubleObject() {}
		virtual void show();
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN op);
		virtual bool isTrue();
	};
}

#endif // __ICE_OBJECT_H__