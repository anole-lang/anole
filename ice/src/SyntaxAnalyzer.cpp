#include "SyntaxAnalyzer.h"

SyntaxAnalyzer::SyntaxAnalyzer(std::vector<Token> &tokens) : tokens(tokens)
{
    iToken = tokens.begin();
    genNode[Symbol::stmt] = [&](){
        return nullptr;
    };
    genNode[Symbol::var_decl_or_func_decl] = [&](){
        return nullptr;
    };
    genNode[Symbol::if_else] = [&](){
        return nullptr;
    };
    genNode[Symbol::expr] = [&](){
        return nullptr;
    };
    genNode[Symbol::return_stmt] = [&](){
        return nullptr;
    };
    genNode[Symbol::var_decl_or_func_decl_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::block] = [&](){
        return nullptr;
    };
    genNode[Symbol::block_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::var_decl_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::func_decl_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::func_decl_rest] = [&](){
        return nullptr;
    };
    genNode[Symbol::func_decl_args] = [&](){
        return nullptr;
    };
    genNode[Symbol::func_decl_args_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::ident] = [&](){
        return nullptr;
    };
    genNode[Symbol::numeric] = [&](){
        return nullptr;
    };
    genNode[Symbol::factor_rest] = [&](){
        return nullptr;
    };
    genNode[Symbol::factor] = [&](){
        return nullptr;
    };
    genNode[Symbol::term] = [&](){
        return nullptr;
    };
    genNode[Symbol::term_rest] = [&](){
        return nullptr;
    };
    genNode[Symbol::method_call_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::call_args] = [&](){
        return nullptr;
    };
    genNode[Symbol::call_args_tail] = [&](){
        return nullptr;
    };
    genNode[Symbol::comparison] = [&](){
        return nullptr;
    };
    genNode[Symbol::if_else_tail] = [&](){
        return nullptr;
    };
}

Node *SyntaxAnalyzer::getNode()
{
    Node *node = genNode[Symbol::stmt]();
    return node;
}