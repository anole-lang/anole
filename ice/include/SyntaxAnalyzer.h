#ifndef __SYNTAX_ANALYZER_H__
#define __SYNTAX_ANALYZER_H__

#include <iostream>
#include <vector>
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
	: var_decl | func_decl 
    | if_else
	| expr
	| return_stmt
	;

return_stmt
	: TRETURN expr
	;

block
	: TLBRACE stmts TRBRACE
	| TLBRACE TRBRACE
	;

var_decl
	: ident TASSIGN expr
	;

func_decl
	: TAT ident TLPAREN func_decl_args TRPAREN func_decl_tail
	;

func_decl_tail
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
	;

expr
	: factor factor_rest
	| TLPAREN expr TRPAREN
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
	: TCEQ | TEXC TCEQ | TCLT | TCLT TCEQ | TCGT | TCGT TCEQ
	;

if_else
	: TIF expr block if_else_tail
	;
	
if_else_tail
	: 
	| TELSE block
	;
*/

class SyntaxAnalyzer
{
private:
	std::vector<Token> &tokens;
	
public:
    SyntaxAnalyzer(std::vector<Token> &tokens):tokens(tokens) {}
    Stmt *getNode();
};

#endif //__SYNTAX_ANALYZER_H__