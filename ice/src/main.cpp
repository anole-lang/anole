#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "Interpreter.h"

using namespace Ice;

int main(int argc, char *argv[])
{
	Interpreter interpreter;
	std::ifstream fin;
	if (argc > 1)
	{
		fin.open(argv[1]);
		if (fin.bad())
		{
			std::cout << "cannot open file " << argv[1] << std::endl;
			exit(0);
		}
		std::cin.rdbuf(fin.rdbuf());
	}
	interpreter.run();
	return 0;
}