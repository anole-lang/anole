#ifndef __ICE_OBJECT_H__
#define __ICE_OBJECT_H__

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cctype>
#include <functional>
#include <cstring>
#include "Token.h"
#include "Coderun.h"

using std::cout;
using std::string;
using std::vector;
using std::function;
using std::shared_ptr;
using std::make_shared;
using std::to_string;
using std::dynamic_pointer_cast;
using std::unordered_map;

using TOKEN = Ice::Token::TOKEN;

namespace Ice
{
	class BlockExpr;
	class VariableDeclarationStmt;
	typedef vector<shared_ptr<VariableDeclarationStmt>> VariableList;
	class IdentifierExpr;
	typedef vector<shared_ptr<IdentifierExpr>> IdentifierList;

	class IceObject;
	typedef vector<shared_ptr<IceObject>> Objects;

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
			LIST,
			DICT
		} type;
		virtual ~IceObject() {}

		virtual void show() {}
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op) 
		{ 
			return nullptr; 
		}
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN) 
		{ 
			return nullptr;
		}
		virtual bool isTrue() = 0;
		virtual string toStr() = 0;

		bool isInstance();
		bool isTraversable();
	};

	class IceFunctionObject : public IceObject
	{
	public:

		VariableList argDecls;
		shared_ptr<BlockExpr> block;

		IceFunctionObject(const VariableList &, shared_ptr<BlockExpr>);
		virtual ~IceFunctionObject() {}

		virtual bool isTrue()
		{ 
			return true; 
		}
		virtual string toStr() 
		{ 
			return "function"; 
		}
	};

	class IceBuiltInFunctionObject : public IceObject
	{
	public:

		function<shared_ptr<IceObject>(Objects)> func;
		IceBuiltInFunctionObject(function<shared_ptr<IceObject>(Objects)>);
		virtual ~IceBuiltInFunctionObject() {}

		virtual bool isTrue() 
		{ 
			return true; 
		}
		virtual string toStr() 
		{ 
			return "built-in function";
		}
	};

	class IceClassObject : public IceObject
	{
	public:

		IdentifierList bases;
		shared_ptr<BlockExpr> block;

		IceClassObject(const IdentifierList &, shared_ptr<BlockExpr>);
		virtual ~IceClassObject() {}

		virtual bool isTrue() 
		{ 
			return true;
		}
		virtual string toStr() 
		{ 
			return "Class"; 
		}
	};

	class IceInstanceObject : public IceObject
	{
	public:

		shared_ptr<Env> top;

		IceInstanceObject();
		IceInstanceObject(shared_ptr<Env> &);
		virtual ~IceInstanceObject() {}

		virtual void show() 
		{ 
			cout << "an instance"; 
		}
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{ 
			return true;
		}
		virtual string toStr() 
		{ 
			return "Instance"; 
		}
	};

	class IceNoneObject : public IceObject
	{
	public:

		IceNoneObject();
		virtual ~IceNoneObject() {}

		virtual shared_ptr<IceObject> unaryOperate(TOKEN op) 
		{ 
			return make_shared<IceNoneObject>(); 
		}
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN) 
		{ 
			return make_shared<IceNoneObject>();
		}
		virtual bool isTrue()
		{
			return false; 
		}
		virtual string toStr() 
		{ 
			return "None";
		}
	};

	class IceIntegerObject : public IceObject
	{
	public:

		long value;

		IceIntegerObject(long value);
		virtual ~IceIntegerObject() {}

		virtual void show() { cout << value; }
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{ 
			return value != 0;
		}
		virtual string toStr() 
		{ 
			return to_string(value); 
		}
	};

	class IceDoubleObject : public IceObject
	{
	public:

		double value;

		IceDoubleObject(double value);
		virtual ~IceDoubleObject() {}

		virtual void show()
		{
			cout << value; 
		}
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return value != 0.0; 
		}
		virtual string toStr() 
		{ 
			return to_string(value); 
		}
	};

	class IceBooleanObject : public IceObject
	{
	public:

		bool value;

		IceBooleanObject(bool value);
		virtual ~IceBooleanObject() {}

		virtual void show();
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{
			return value; 
		}
		virtual string toStr() 
		{ 
			return to_string(value);
		}
	};

	class IceStringObject : public IceInstanceObject
	{
	private:

		string raw_value;
		void genBuiltInMethods();

	public:

		string value;

		IceStringObject(string value);
		virtual ~IceStringObject() {}

		virtual void show()
		{ 
			cout << '\"' << raw_value << "\""; 
		}
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return value != "";
		}
		virtual string toStr() 
		{ 
			return value; 
		}

		shared_ptr<IceObject> getByIndex(shared_ptr<IceObject>);
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
		virtual shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual shared_ptr<IceObject> binaryOperate(shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return objects.size() != 0; 
		}
		virtual string toStr() 
		{ 
			return "list"; 
		}

		shared_ptr<IceObject> getByIndex(shared_ptr<IceObject>);
		void setByIndex(shared_ptr<IceObject>, shared_ptr<IceObject>);
	};

	class KeyObject
	{
	public:

		shared_ptr<IceObject> obj;
		KeyObject(shared_ptr<IceObject> obj) : obj(obj) {}

		size_t hashValue() const;
	};

	class KeyHash
	{
	public:

		size_t operator()(const KeyObject &key) const
		{
			return key.hashValue();
		}
	};

	class KeyEqual
	{
	public:

		bool operator()(const KeyObject &key1, const KeyObject &key2) const
		{
			return dynamic_pointer_cast<IceBooleanObject>(key1.obj->binaryOperate(key2.obj, TOKEN::TCEQ))->value;
		}
	};

	class IceDictObject : public IceInstanceObject
	{
	private:

		void genBuiltInMethods();

	public:

		unordered_map<KeyObject, shared_ptr<IceObject>, KeyHash, KeyEqual> objects_map;

		IceDictObject();
		virtual ~IceDictObject() {}
		
		virtual void show();
		virtual bool isTrue() 
		{ 
			return objects_map.size() != 0; 
		}
		virtual string toStr() 
		{ 
			return "dict";
		}

		shared_ptr<IceObject> getByIndex(shared_ptr<IceObject>);
		void setByIndex(shared_ptr<IceObject>, shared_ptr<IceObject>);
	};
}

#endif // __ICE_OBJECT_H__