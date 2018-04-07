#include <iostream>
#include <vector>
#include "Node.h"
#include "Token.h"
#include "LexicalAnalyzer.h"

/* Grammer
program	
	: stmts
	;

stmts
	: stmt
	| stmts stmt
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
	: TAT ident TLPAREN func_decl_args TRPAREN TASSIGN expr
	| TAT ident TLPAREN func_decl_args TRPAREN block
	;

func_decl_args
	: 
	| ident 
	| func_decl_args TCOMMA ident 
	;

ident
	: TIDENTIFIER
	;

numeric	
	: TINTEGER
	;

expr
	: ident TLPAREN call_args TRPAREN
	| ident
	| numeric
	| expr TADD expr
	| expr TSUB expr
	| expr TMUL expr
	| expr TDIV expr
	| expr TMOD expr
	| expr comparison expr
	| TLPAREN expr TRPAREN
	;

call_args
	:
	| expr
	| call_args TCOMMA expr
	;

comparison
	: TCEQ | TEXC TCEQ | TCLT | TCLT TCEQ | TCGT | TCGT TCEQ
	;

if_else
	: TIF expr block
	| TIF expr block TELSE block
    ;
*/

class SyntaxAnalyzer
{
private:
    std::vector<Token> &tokens;
public:
    SyntaxAnalyzer(std::vector<Token>&):tokens(tokens) {}
    Node *getNode();
};