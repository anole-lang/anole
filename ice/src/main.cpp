#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "Interpreter.h"

using namespace Ice;

namespace Ice
{
	Interpreter interpreter;
}

int main(int argc, char *argv[])
{
	if (argc > 1) interpreter.run(argv[1]);
	else interpreter.run();
	return 0;
}