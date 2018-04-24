#ifndef __CODERUN_H__
#define __CODERUN_H__

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include "IceObject.h"
#include "Node.h"

namespace Ice
{
	class Env : public std::enable_shared_from_this<Env>
	{
	private:
		std::map<std::string, std::shared_ptr<IceObject>> objects;
		std::map<std::string, std::shared_ptr<FunctionDeclarationStmt>> functions;

		std::shared_ptr<IceObject> returnValue;
		bool breakStatus;
		bool continueStatus;

		void genPrintFunction();

	public:
		std::shared_ptr<Env> prev;
		Env(std::shared_ptr<Env> prev) : prev(prev) { returnValue = nullptr; breakStatus = false; continueStatus = false; }

		void genBuildInFunction();

		void put(std::string &, std::shared_ptr<IceObject>);
		std::shared_ptr<IceObject> getObject(std::string &);

		void put(std::string &, std::shared_ptr<FunctionDeclarationStmt>);
		std::shared_ptr<FunctionDeclarationStmt> getFunction(std::string &);
		
		void setReturnValue(std::shared_ptr<IceObject> returnValue) { this->returnValue = returnValue; }
		std::shared_ptr<IceObject> getReturnValue() { return returnValue; }

		void setBreakStatus(bool status) { breakStatus = status; }
		bool getBreakStatus() { return breakStatus; }

		void setContinueStatus(bool status) { continueStatus = status; }
		bool getContinueStatus() { return continueStatus; }
	};
}

#endif // __CODERUN_H__