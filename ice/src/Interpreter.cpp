#include "Interpreter.h"

namespace Ice
{
Interpreter::Interpreter()
{
    std::cout << "      /////////\n"
                 "        ///    ///////// /////////\n"
                 "       ///    ///       ///   ///    Version 0.0.1 \n"
                 "      ///    ///       /////////     http://www.jusot.com/ice\n"
                 "     ///    ///       ///\n"
                 " ///////// ///////// /////////\n"
              << std::endl;
    top = std::make_shared<Env>(nullptr);
    block = std::make_shared<BlockExpr>();
}

void Interpreter::run()
{
    while (true)
    {
        std::cout << ">> ";
        
        auto &tokens = lexicalAnalyzer.getTokens();
        auto node = syntaxAnalyzer.getNode(tokens);

        auto obj = (node == nullptr) ? nullptr : node->runCode(top);
        if (obj != nullptr) obj->show();

        block->statements.push_back(std::dynamic_pointer_cast<Stmt>(node));
    }
}
}