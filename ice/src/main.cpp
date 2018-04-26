#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include "Interpreter.h"

using namespace Ice;

int main(int argc, char *argv[])
{
	Interpreter interpreter;
	if (argc > 1) interpreter.run(argv[1]);
	else interpreter.run();
	return 0;
}