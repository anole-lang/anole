#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include "IceObject.h"

namespace Ice
{
	class Env : public std::enable_shared_from_this<Env>
	{
	private:
		std::map<std::string, std::shared_ptr<IceObject>> objects;
		std::shared_ptr<IceObject> returnValue;

	protected:
		std::shared_ptr<Env> prev;

	public:
		Env(std::shared_ptr<Env> prev) : prev(prev) {}

		void put(std::string &, std::shared_ptr<IceObject>);
		std::shared_ptr<IceObject> getObject(std::string &);
		
		void setReturnValue(std::shared_ptr<IceObject> returnValue) { this->returnValue = returnValue; }
		std::shared_ptr<IceObject> getReturnValue() { return returnValue; }
	};
}

#endif // __CODERUN_H__