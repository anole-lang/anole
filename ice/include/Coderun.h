#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <cstring>

using std::string;
using std::map;
using std::shared_ptr;
using std::enable_shared_from_this;

namespace Ice
{
	class IceObject;

	class Env : public enable_shared_from_this<Env>
	{
	private:
		map<string, shared_ptr<IceObject>> objects;

		shared_ptr<IceObject> returnValue;
		bool breakStatus;
		bool continueStatus;

		void garbageCollect(std::string name);

	public:
		shared_ptr<Env> prev;
		Env(shared_ptr<Env> prev) : prev(prev) 
		{ 
			returnValue = nullptr; 
			breakStatus = false; 
			continueStatus = false; 
		}

		void setReturnValue(shared_ptr<IceObject> returnValue) { this->returnValue = returnValue; }
		shared_ptr<IceObject> getReturnValue() { return returnValue; }

		void setBreakStatus(bool status) { breakStatus = status; }
		bool getBreakStatus() { return breakStatus; }

		void setContinueStatus(bool status) { continueStatus = status; }
		bool getContinueStatus() { return continueStatus; }

		void genBuildInFunctions();

		void put(string, shared_ptr<IceObject>);
		void replace(std::string, shared_ptr<IceObject>);
		shared_ptr<IceObject> getObject(string);

		void garbageCollection();
	};
}

#endif // __CODERUN_H__