#include "Interpreter.h"

Interpreter::Interpreter()
{
    std::cout << "      /////////\n"
                 "        ///    ///////// /////////\n"
                 "       ///    ///       ///   ///    Version 0.0.1 \n"
                 "      ///    ///       /////////     http://www.jusot.com/ice\n"
                 "     ///    ///       ///\n"
                 " ///////// ///////// /////////\n" << std::endl;
    top = new Env(nullptr);
    block = new BlockExpr();
}

void Interpreter::run()
{
    while (true)
    {
        std::string line;
        std::cout << ">> ";

        getline(std::cin, line);
        line += '\n';

        LexicalAnalyzer lexAnalyzer(line);
        auto &tokens = lexAnalyzer.getTokens();

        SyntaxAnalyzer syntaxAnalyzer(tokens);
        auto node = syntaxAnalyzer.getNode();

        auto obj = node->runCode(top);
        if ( obj != nullptr) obj->show();

        block->statements.push_back(dynamic_cast<Stmt *>(node));
    }
}