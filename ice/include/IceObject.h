#ifndef __ICE_OBJECT_H__
#define __ICE_OBJECT_H__

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <functional>
#include <cstring>
#include "Token.h"
#include "Coderun.h"

namespace Ice
{
	class BlockExpr;
	class VariableDeclarationStmt;
	typedef std::vector<std::shared_ptr<VariableDeclarationStmt>> VariableList;
	class IdentifierExpr;
	typedef std::vector<std::shared_ptr<IdentifierExpr>> IdentifierList;

	class IceObject;
	typedef std::vector<std::shared_ptr<IceObject>> Objects;

	class IceObject
	{
	public:
		enum class TYPE
		{
			INT,
			DOUBLE,
			BOOLEAN,
			STRING,
			FUNCTION,
			BUILT_IN_FUNCTION,
			CLASS,
			INSTANCE,
			NONE,
			LIST
		} type;
		virtual ~IceObject() {}

		virtual void show() {}
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op) { return nullptr; }
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN) { return nullptr; }
		virtual bool isTrue() = 0;
		virtual std::string toStr() = 0;
	};

	class IceFunctionObject : public IceObject
	{
	public:
		VariableList argDecls;
		std::shared_ptr<BlockExpr> block;

		IceFunctionObject(const VariableList &, std::shared_ptr<BlockExpr>);
		virtual ~IceFunctionObject() {}

		virtual bool isTrue() { return true; }
		virtual std::string toStr() { return "function"; }
	};

	class IceBuiltInFunctionObject : public IceObject
	{
	public:
		std::function<std::shared_ptr<IceObject>(Objects)> func;
		IceBuiltInFunctionObject(std::function<std::shared_ptr<IceObject>(Objects)>);
		virtual ~IceBuiltInFunctionObject() {}

		virtual bool isTrue() { return true; }
		virtual std::string toStr() { return "built-in function"; }
	};

	class IceClassObject : public IceObject
	{
	public:
		IdentifierList bases;
		std::shared_ptr<BlockExpr> block;

		IceClassObject(const IdentifierList &, std::shared_ptr<BlockExpr>);
		virtual ~IceClassObject() {}

		virtual bool isTrue() { return true; }
		virtual std::string toStr() { return "Class"; }
	};

	class IceInstanceObject : public IceObject
	{
	public:
		std::shared_ptr<Env> top;

		IceInstanceObject();
		IceInstanceObject(std::shared_ptr<Env> &);
		virtual ~IceInstanceObject() {}

		virtual void show() { std::cout << "an instance"; }
		virtual bool isTrue() { return true; }
		virtual std::string toStr() { return "Instance"; }
	};

	class IceNoneObject : public IceObject
	{
	public:
		IceNoneObject();
		virtual ~IceNoneObject() {}

		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op) { return std::make_shared<IceNoneObject>(); };
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN) { return std::make_shared<IceNoneObject>(); }
		virtual bool isTrue() { return false; }
		virtual std::string toStr() { return "None"; }
	};

	class IceIntegerObject : public IceObject
	{
	public:
		long value;

		IceIntegerObject(long value);
		virtual ~IceIntegerObject() {}

		virtual void show() { std::cout << value; }
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue() { return value != 0; }
		virtual std::string toStr() { return std::to_string(value); }
	};

	class IceDoubleObject : public IceObject
	{
	public:
		double value;

		IceDoubleObject(double value);
		virtual ~IceDoubleObject() {}

		virtual void show() { std::cout << value; }
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue() { return value != 0.0; }
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
		virtual bool isTrue() { return value; }
		virtual std::string toStr() { return std::to_string(value); }
	};

	class IceStringObject : public IceInstanceObject
	{
	private:
		std::string raw_value;
		void genBuiltInMethods();

	public:
		std::string value;

		IceStringObject(std::string value);
		virtual ~IceStringObject() {}

		virtual void show() { std::cout << '\"' << raw_value << "\""; }
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue() { return value != ""; }
		virtual std::string toStr() { return value; }

		std::shared_ptr<IceObject> getByIndex(std::shared_ptr<IceObject>);
	};

	class IceListObject : public IceInstanceObject
	{
	private:
		void genBuiltInMethods();

	public:
		Objects objects;

		IceListObject();
		IceListObject(Objects objects);
		virtual ~IceListObject() {}

		virtual void show();
		virtual std::shared_ptr<IceObject> unaryOperate(Token::TOKEN op);
		virtual std::shared_ptr<IceObject> binaryOperate(std::shared_ptr<IceObject>, Token::TOKEN);
		virtual bool isTrue() { return objects.size() != 0; }
		virtual std::string toStr() { return "list"; }

		std::shared_ptr<IceObject> getByIndex(std::shared_ptr<IceObject>);
		void setByIndex(std::shared_ptr<IceObject>, std::shared_ptr<IceObject>);
	};
}

#endif // __ICE_OBJECT_H__