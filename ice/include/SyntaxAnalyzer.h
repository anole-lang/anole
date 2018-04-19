#ifndef __SYNTAX_ANALYZER_H__
#define __SYNTAX_ANALYZER_H__

#include <iostream>
#include <vector>
#include <stack>
#include <iterator>
#include <functional>
#include <map>
#include "Token.h"
#include "Node.h"

/* LL(1) Grammer
program	
	: stmts
	;

stmts
	:
	| stmt stmts
	;

stmt
	: var_decl_or_func_decl 
    | if_else
	| expr
	| return_stmt
	;

var_decl_or_func_decl
	: TAT ident var_decl_or_func_decl_tail
	;

var_decl_or_func_decl_tail
	: var_decl_tail
	| func_decl_tail
	;

return_stmt
	: TRETURN expr
	;

block
	: TLBRACE block_tail
	;

block_tail
	: stmts TRBRACE
	| TRBRACE
	;

var_decl_tail
	: TASSIGN expr
	;

func_decl_tail
	: TLPAREN func_decl_args TRPAREN func_decl_rest
	;

func_decl_rest
	: TASSIGN expr
	| block
	;

func_decl_args
	: 
	| ident func_decl_args_tail
	;

func_decl_args_tail
	:
	| TCOMMA ident func_decl_args_tail

ident
	: TIDENTIFIER
	;

numeric	
	: TINTEGER
	| TDOUBLE
	;

expr
	: factor factor_rest
	;

factor_rest
	:
	| TADD factor factor_rest
	| TSUB factor factor_rest
	| comparison factor
	;

factor
	: term term_rest
	;

term
	: ident method_call_tail
	| numeric
	| TLPAREN expr TRPAREN
	;

term_rest
	:
	| TMUL term term_rest
	| TDIV term term_rest
	| TMOD term term_rest
	;

method_call_tail
	:
	| TLPAREN call_args TRPAREN
	;

call_args
	:
	| expr call_args_tail
	;

call_args_tail
	:
	| TCOMMA expr call_args_tail
	;

comparison
	: TCEQ | TEXC | TCLT | TCGT
	;

if_else
	: TIF expr block if_else_tail
	;
	
if_else_tail
	: 
	| TELSE block
	;
*/

/* Predictive Parser

| Non-terminal symbols  	 | Input symbols
|					    	 | TAT 										| TIF 							| TELSE 		| TWHILE	| TRETURN 			| TIDENTIFIER 					| TINTEGER				| TASSIGN			| TCOMMA 							 | TLPAREN 											| TRPAREN	| TLBRACE 				| TRBRACE	| TADD 						| TSUB 						| TMUL 					| TDIV 					| TMOD 					| TCEQ		| TEXC		| TCLT		| TCGT
| stmts						 | ->stmt stmts								| ->stmt stmts					|				| TODO		| ->stmt stmts		| ->stmt stmts					| ->stmt stmts			|					|									 | ->stmt stmts 									|			|						| ->ε		|							|							|						|						|						|			|			|			|
| stmt				    	 | ->var_decl_or_func_decl					| ->if_else 					|				| TODO		| ->return_stmt		| ->expr						| ->expr				| 					| 									 | ->expr											|			|						|			|							|							|						|						|						|			|			|			|
| var_decl_or_func_decl 	 | ->TAT ident var_decl_or_func_decl_tail	|								|				|			|					|								|						|					|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| if_else					 |											| ->TIF expr block if_else_tail |				|			|					|								|						|					|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| expr						 |											|								|				|			|					| ->factor factor_rest			| ->factor factor_rest  |					|									 | ->TLPAREN expr TRPAREN							|			|						|			|							|							|						|						|						|			|			|			|
| return_stmt				 |											|								|				|			| ->TRETURN expr	|								|						|					|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| var_decl_or_func_decl_tail |											|								|				|			|					|								|						| ->var_decl_tail	|									 | ->func_decl_tail									|			|						|			|							|							|						|						|						|			|			|			|			
| block						 |											|								|				|			|					|								|						|					|									 |													|			| ->TLBRACE block_tail	|			|							|							|						|						|						|			|			|			|
| block_tail				 | ->stmts TRBRACE							| ->stmts TRBRACE				|				| TODO		| ->stmts TRBRACE	| ->stmts TRBRACE				| ->stmts TRBRACE		|					|									 | ->stmts TRBRACE									|			|						| ->TRBRACE	|							|							|						|						|						|			|			|			|
| var_decl_tail				 |											|								|				|			|					|								|						| ->TASSIGN expr	|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| func_decl_tail			 |											|								|				|			|					|								|						|					|									 | ->TLPAREN func_decl_args TRPAREN func_decl_rest	|			|						|			|							|							|						|						|						|			|			|			|
| func_decl_rest			 |											|								|				|			|					|								|						| ->TASSIGN expr	|									 |													|			| ->block				|			|							|							|						|						|						|			|			|			|
| func_decl_args			 |											|								|				|			|					| ->ident func_decl_args_tail	|						|					|									 |													| ->ε		|						|			|							|							|						|						|						|			|			|			|
| func_decl_args_tail		 |											|								|				|			|					|								|						|					| ->TCOMMA ident func_decl_args_tail |													| ->ε		|						|			|							|							|						|						|						|			|			|			|
| ident						 |											|								|				|			|					| ->TIDENTIFIER					| 						|					|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| numeric					 |											|								|				|			|					|								| ->TINTEGER 			|					|									 |													|			|						|			|							|							|						|						|						|			|			|			|
| factor_rest				 |											|								|				|			|					|								|						|					| ->ε								 |													| ->ε		|						| ->ε		| ->TADD factor factor_rest	| ->TSUB factor factor_rest	|						|						|						|			|			|			|
| factor					 |											|								|				|			|					| ->term term_test				| ->term term_rest 		|					|									 | ->term term_rest									|			|						|			|							|							|						|						|						|			|			|			|
| term						 |											|								|				|			|					| ->ident method_call_tail 		| ->numeric				|					|									 | ->TLPAREN expr TRPAREN							|			|						|			|							|							|						|						|						|			|			|			|
| term_rest					 |											|								|				|			|					|								|						|					| ->ε								 |													| ->ε		|						| ->ε		|							|							| ->TMUL term term_rest	| ->TDIV term term_rest	| ->TMOD term term_rest	|			|			|			|
| method_call_tail			 |											|								|				|			|					|								|						|					| ->ε								 | ->TLPAREN call_args TRPAREN						| ->ε		|						| ->ε		|							|							|						|						|						|			|			|			|
| call_args					 |											|								|				|			|					| ->expr call_args_tail			| ->expr call_args_tail	|					|									 | ->expr call_args_tail							| ->ε		|						|			|							|							|						|						|						|			|			|			|
| call_args_tail			 |											|								|				|			|					|								|						|					| TCOMMA expr call_args_tail		 |													| ->ε		|						|			|							|							|						|						|						|			|			|			|
| comparison				 |											|								|				|			|					|								|						|					|									 |													|			|						|			|							|							|						|						|						| ->TCEQ	| ->TEXC	| ->TCLT	| ->TCGT
| if_else_tail				 | ->ε										| ->ε							| ->TELSE block |			| ->ε				| ->ε							| ->ε					|					|									 |													|			|						| ->ε		|							|							|						|						|						|			|			|			|
*/

class SyntaxAnalyzer
{
  private:
	std::vector<Token> &tokens;
	std::vector<Token>::iterator iToken;
	enum class Symbol
	{
		stmts,
		stmt,
		var_decl_or_func_decl,
		if_else,
		expr,
		return_stmt,
		var_decl_or_func_decl_tail,
		block,
		block_tail,
		var_decl_tail,
		func_decl_tail,
		func_decl_rest,
		func_decl_args,
		func_decl_args_tail,
		ident,
		numeric,
		factor_rest,
		factor,
		term,
		term_rest,
		method_call_tail,
		call_args,
		call_args_tail,
		comparison,
		if_else_tail
	};
	std::map<Symbol, std::function<Node *()>> genNode;

  public:
	SyntaxAnalyzer(std::vector<Token> &tokens);
	Node *getNode();
};

#endif //__SYNTAX_ANALYZER_H__