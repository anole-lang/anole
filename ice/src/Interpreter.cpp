#include "interpreter.h"

Interpreter::Interpreter()
{
    std::cout << "Ice 0.0.1" << std::endl;
}

void Interpreter::run()
{
    while (true)
    {
        std::string line;
        std::cout << ">> ";
        getline(std::cin, line);
        Parser parser(line);
        auto tokens = parser.getTokens();
        for (auto &token: tokens)
        {
            std::cout << '<' << token.token_id << ', ' << token.value << '>' << std::endl; 
        }
    }
}