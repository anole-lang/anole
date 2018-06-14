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

/* Grammer
program
	: stmts
	;

stmts
	:
	| stmt stmts
	;

stmt
	: decl_or_assign
	| class_decl
	| if_else
	| while_stmt
	| do_while_stmt
	| for_stmt
	| expr
	| break_stmt
	| continue_stmt
	| return_stmt
	| using_stmt
	;

decl_or_assign
	: TAT ident other_tail var_decl_or_func_decl_tail
	| var_assign
	;

var_assign
	: TDOT ident TASSIGN expr
	;

other_tail
	: dot_tail
	| TLBRACKET expr TRBRACKET dot_tail
	;

dot_tail
	|
	| TLPAREN func_decl_args TRPAREN
	| TDOT ident other_tail
	;

var_decl_or_func_decl_tail
	: var_decl_tail
	| TLPAREN func_decl_tail
	;

return_stmt
	: TRETURN expr
	;

using_stmt
	: TUSING TIDENTIFIER
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
	: func_decl_args TRPAREN func_decl_rest
	| TRPAREN func_decl_rest
	;

func_decl_rest
	: TASSIGN expr
	| block
	;

func_decl_args
	| ident func_decl_args_tail
	;

func_decl_args_tail
	:
	| TASSIGN expr TCOMMA func_decl_args
	| TCOMMA func_decl_args

ident
	: TIDENTIFIER
	;

numeric
	: TINTEGER
	| TDOUBLE
	;

boolean
	: TTRUE
	| TFALSE
	;

string
	: TSTRING
	;

expr
	: logic_or logic_or_rest
	| TLBRACE expr enum_or_dict
	;

logic_or
	: logic_and logic_and_rest
	;

logic_or_rest
	: 
	| TOR logic_or logic_or_rest
	;

logic_and
	: logic_not
	;

logic_and_rest
	: 
	| TAND logic_or
	;

logic_not
	: cmp cmp_rest
	| TNOT cmp cmp_rest
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
	: ident_or_other
	| numeric
	| boolean
	| string
	| TLPAREN expr TRPAREN
	| TSUB term
	| lambda_expr
	| new_expr
	| match_expr
	| list_expr
	;

term_rest
	:
	| TMUL term term_rest
	| TDIV term term_rest
	| TMOD term term_rest
	;

ident_or_other
	: ident other_tail
	;

other_tail
	:
	| TLPAREN call_args TRPAREN other_tail
	| TDOT ident other_tail
	| TLBRACKET expr TRBRACKET other_tail
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

lambda_expr
	: TAT TLPAREN func_decl_args TRPAREN block method_call_tail
	;

class_decl
	: TATAT ident TLPAREN func_decl_args TRPAREN block
	;

new_expr
	: TNEW ident TLPAREN call_args TRPAREN
	;

enum_or_dict
	: TCOMMA enum_expr
	| TASSIGN expr TCOMMA dict_expr
	;

enum_expr
	: enumerators TRBRACE
	| TRBRACE
	;

enumerators
	| ident enumerators_rest
	;

enumerators_rest
	: 
	| TCOMMA enumerators
	;

dict_expr
	: dictionary TRBRACE
	| TRBRACE
	;

dictionary
	: expr TASSIGN expr dictionary_rest
	;

dictionary_rest
	: 
	| TCOMMA dictionary
	;
	
match_expr
	: TMATCH expr TLBRACE matches TRBRACE (TELSE expr)
	;

matches
	: expr call_args_tail TRET expr matches_rest
	;

matches_rest
	:
	| TCOMMA matches
	;

list_expr
	: TLBRACKET call_args TRBRACKET
	;
*/

using std::shared_ptr;
using std::make_shared;
using std::vector;

typedef shared_ptr<Ice::BlockExpr> BLOCK_EXPR;
typedef shared_ptr<Ice::Stmt> STMT;
typedef shared_ptr<Ice::Expr> EXPR;

namespace Ice
{
	class SyntaxAnalyzer
	{

	private:

		LexicalAnalyzer lexicalAnalyzer;
		vector<Token>::iterator iToken;
		void updateiToken();

		BLOCK_EXPR genStmts();
		BLOCK_EXPR genBlock();
		BLOCK_EXPR genIfElseTail();
		BLOCK_EXPR genFuncDeclRest();

		STMT genStmt();
		STMT genDeclOrAssign();
		STMT genVarAssign();
		STMT genClassDecl();
		STMT genUsingStmt();
		STMT genIfElse();
		STMT genWhileStmt();
		STMT genDoWhileStmt();
		STMT genForStmt();
		STMT genReturnStmt();

		EXPR genExpr();
		EXPR genLogicOr();
		EXPR genLogicAnd();
		EXPR genLogicNot();
		EXPR genCmp();
		EXPR genFactor();
		EXPR genTerm();
		EXPR genIdentOrOther();
		EXPR genIdent();
		EXPR genNumeric();
		EXPR genBoolean();
		EXPR genString();
		EXPR genEnumOrDict();
		EXPR genLambdaExpr();
		EXPR genNewExpr();
		EXPR genMatchExpr();
		EXPR genListExpr();

	public:

		SyntaxAnalyzer() {}
		shared_ptr<Node> getNode();
		shared_ptr<Node> getNode(std::string);
	};
}

#endif //__SYNTAX_ANALYZER_H__