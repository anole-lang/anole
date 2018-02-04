%{
	#include "node.h"
	NBlock *programBlock;

	extern int yylex();
	void yyerror(const char *s) { printf("ERROR: %s\n", s); }
%}

%union{
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NVariableDeclaration*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

%token <string> TIDENTIFIER TINTEGER TDOUBLE
%token <token> TASSIGN

%type <ident> ident
%type <expr> numeric expr
%type <block> program stmts
%type <stmt> stmt var_decl

%start program

%%

program	: stmts { programBlock = $1; }
		;
stmts	: stmt { $$ = new NBlock(); $$->statements.push_back($1); }
		| stmts stmt { $1->statements.push_back($2); }
		;
stmt	: var_decl
		| expr { $$ = new NExpressionStatement(*$1); }
		;
var_decl: ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
		| ident ident TASSIGN expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
		;
ident	: TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
		;
numeric	: TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
		| TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
		;
expr	: ident TASSIGN expr { $$ = new NAssignment(*$1, *$3); }
		| ident { $$ = $1; }
		| numeric
		;

%%