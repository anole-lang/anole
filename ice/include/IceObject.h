#ifndef __ICE_OBJECT_H__
#define __ICE_OBJECT_H__


namespace Ice
{

	using VariableList = ::std::vector<::std::shared_ptr<class VariableDeclarationStmt>>;
	using IdentifierList = ::std::vector<::std::shared_ptr<class IdentifierExpr>> ;
	using Objects = ::std::vector<::std::shared_ptr<class IceObject>>;


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
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op) 
		{ 
			return nullptr; 
		}
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN) 
		{ 
			return nullptr;
		}
		virtual bool isTrue() = 0;
		virtual ::std::string toStr() = 0;

		bool isInstance();
		bool isTraversable();
	};

	class IceFunctionObject : public IceObject
	{
	public:

		VariableList argDecls;
		::std::shared_ptr<class BlockExpr> block;

		IceFunctionObject(const VariableList &, ::std::shared_ptr<BlockExpr>);
		virtual ~IceFunctionObject() {}

		virtual bool isTrue()
		{ 
			return true; 
		}
		virtual ::std::string toStr() 
		{ 
			return "::std::function"; 
		}
	};

	class IceBuiltInFunctionObject : public IceObject
	{
	public:

		::std::function<::std::shared_ptr<IceObject>(Objects)> func;
		IceBuiltInFunctionObject(::std::function<::std::shared_ptr<IceObject>(Objects)>);
		virtual ~IceBuiltInFunctionObject() {}

		virtual bool isTrue() 
		{ 
			return true; 
		}
		virtual ::std::string toStr() 
		{ 
			return "built-in ::std::function";
		}
	};

	class IceClassObject : public IceObject
	{
	public:

		IdentifierList bases;
		::std::shared_ptr<BlockExpr> block;

		IceClassObject(const IdentifierList &, ::std::shared_ptr<BlockExpr>);
		virtual ~IceClassObject() {}

		virtual bool isTrue() 
		{ 
			return true;
		}
		virtual ::std::string toStr() 
		{ 
			return "Class"; 
		}
	};

	class IceInstanceObject : public IceObject
	{
	public:

		::std::shared_ptr<Env> top;

		IceInstanceObject();
		IceInstanceObject(::std::shared_ptr<Env> &);
		virtual ~IceInstanceObject() {}

		virtual void show() 
		{ 
			::std::cout << "an instance"; 
		}
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{ 
			return true;
		}
		virtual ::std::string toStr() 
		{ 
			return "Instance"; 
		}
	};

	class IceNoneObject : public IceObject
	{
	public:

		IceNoneObject();
		virtual ~IceNoneObject() {}

		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op) 
		{ 
			return ::std::make_shared<IceNoneObject>(); 
		}
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN) 
		{ 
			return ::std::make_shared<IceNoneObject>();
		}
		virtual bool isTrue()
		{
			return false; 
		}
		virtual ::std::string toStr() 
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

		virtual void show() { ::std::cout << value; }
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{ 
			return value != 0;
		}
		virtual ::std::string toStr() 
		{ 
			return ::std::to_string(value); 
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
			::std::cout << value; 
		}
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return value != 0.0; 
		}
		virtual ::std::string toStr() 
		{ 
			return ::std::to_string(value); 
		}
	};

	class IceBooleanObject : public IceObject
	{
	public:

		bool value;

		IceBooleanObject(bool value);
		virtual ~IceBooleanObject() {}

		virtual void show();
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue() 
		{
			return value; 
		}
		virtual ::std::string toStr() 
		{ 
			return ::std::to_string(value);
		}
	};

	class IceStringObject : public IceInstanceObject
	{
	private:

		::std::string raw_value;
		void genBuiltInMethods();

	public:

		::std::string value;

		IceStringObject(::std::string value);
		virtual ~IceStringObject() {}

		virtual void show()
		{ 
			::std::cout << '\"' << raw_value << "\""; 
		}
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return value != "";
		}
		virtual ::std::string toStr() 
		{ 
			return value; 
		}

		::std::shared_ptr<IceObject> getByIndex(::std::shared_ptr<IceObject>);
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
		virtual ::std::shared_ptr<IceObject> unaryOperate(TOKEN op);
		virtual ::std::shared_ptr<IceObject> binaryOperate(::std::shared_ptr<IceObject>, TOKEN);
		virtual bool isTrue()
		{ 
			return objects.size() != 0; 
		}
		virtual ::std::string toStr() 
		{ 
			return "list"; 
		}

		::std::shared_ptr<IceObject> getByIndex(::std::shared_ptr<IceObject>);
		void setByIndex(::std::shared_ptr<IceObject>, ::std::shared_ptr<IceObject>);
	};

	class KeyObject
	{
	public:

		::std::shared_ptr<IceObject> obj;
		KeyObject(::std::shared_ptr<IceObject> obj) : obj(obj) {}

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
			return ::std::dynamic_pointer_cast<IceBooleanObject>(key1.obj->binaryOperate(key2.obj, TOKEN::TCEQ))->value;
		}
	};

	class IceDictObject : public IceInstanceObject
	{
	private:

		void genBuiltInMethods();

	public:

		::std::unordered_map<KeyObject, ::std::shared_ptr<IceObject>, KeyHash, KeyEqual> objects_map;

		IceDictObject();
		virtual ~IceDictObject() {}
		
		virtual void show();
		virtual bool isTrue() 
		{ 
			return objects_map.size() != 0; 
		}
		virtual ::std::string toStr() 
		{ 
			return "dict";
		}

		::std::shared_ptr<IceObject> getByIndex(::std::shared_ptr<IceObject>);
		void setByIndex(::std::shared_ptr<IceObject>, ::std::shared_ptr<IceObject>);
	};
}

#endif // __ICE_OBJECT_H__