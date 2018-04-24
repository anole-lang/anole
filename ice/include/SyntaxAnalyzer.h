#ifndef __SYNTAX_ANALYZER_H__
#define __SYNTAX_ANALYZER_H__

#include <iostream>
#include <vector>
#include <memory>
#include <stack>
#include <iterator>
#include <functional>
#include <map>
#include "Token.h"
#include "Node.h"
#include "LexicalAnalyzer.h"

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
	| while_stmt
	| do_while_stmt
	| for_stmt
	| expr
	| break_stmt
	| continue_stmt
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
	: cmp cmp_rest
	;

cmp
	: factor fatcor_rest
	;

cmp_rest
	:
	| TCEQ cmp cmp_rest
	| TCNE cmp cmp_rest
	| TCLT cmp cmp_rest
	| TCLE cmp cmp_rest
	| TCGT cmp cmp_rest
	| TCGE cmp cmp_rest
	;

factor
	: term term_rest
	;

factor_rest
	:
	| TADD factor factor_rest
	| TSUB factor factor_rest
	| comparison factor
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
	: TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
	;

if_else
	: TIF expr block if_else_tail
	;

if_else_tail
	:
	| TELSE block
	;

while_stmt
	: TWHILE expr block
	;

do_while_stmt
	: TDO block TWHILE expr
	;

for_stmt
	: TFOR expr TTO expr block
	;
*/

namespace Ice
{
	class SyntaxAnalyzer
	{
	private:
		LexicalAnalyzer lexicalAnalyzer;
		std::vector<Token>::iterator iToken;
		enum class Symbol
		{
			stmts,
			stmt,
			var_decl_or_func_decl,
			if_else,
			while_stmt,
			do_while_stmt,
			for_stmt,
			expr,
			return_stmt,
			block,
			block_tail,
			var_decl_tail,
			func_decl_tail,
			func_decl_rest,
			func_decl_args,
			func_decl_args_tail,
			ident,
			numeric,
			cmp,
			factor,
			term,
			method_call_tail,
			call_args,
			call_args_tail,
			comparison,
			if_else_tail
		};
		std::map<Symbol, std::function<std::shared_ptr<Node>()>> genNode;

	public:
		SyntaxAnalyzer();
		std::shared_ptr<Node> getNode();
	};
}

#endif //__SYNTAX_ANALYZER_H__