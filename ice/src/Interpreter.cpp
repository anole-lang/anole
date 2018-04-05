#include "interpreter.h"

Interpreter::Interpreter()
{
    std::cout << "Ice 0.0.1" << std::endl;
}

Interpreter::~Interpreter()
{

}

void Interpreter::run()
{
    while (true)
    {
        std::string line;
        std::cout << ">>" << std::endl;
        getline(std::cin, line);
    }
}