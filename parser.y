%{
	#include "node.h"
	BlockExprAST *programBlock;

	extern int yylex();
	void yyerror(const char *s) { printf("ERROR: %s\n", s); }
%}

%union{
	Node *node;
	BlockExprAST *block;
	ExprAST *expr;
	StmtAST *stmt;
	IdentifierExprAST *ident;
	VariableDeclarationStmtAST *var_decl;
	std::vector<VariableDeclarationStmtAST*> *varvec;
    std::vector<ExprAST*> *exprvec;
    std::string *string;
    int token;

	PrintStmtAST *print;
}

%token <string> TIDENTIFIER TINTEGER TDOUBLE
%token <token> TASSIGN
%token <token> TLPAREN TRPAREN
%token <token> TADD TSUB TMUL TDIV TMOD
%token <token> TPRINT

%type <ident> ident
%type <expr> numeric expr
%type <block> program stmts
%type <stmt> stmt var_decl
%type <token> b_op
%type <stmt> print

%left TADD TSUB
%left TMUL TDIV TMOD

%start program

%%

program	: stmts { programBlock = $1; }
		;
stmts	: stmt { $$ = new BlockExprAST(); $$->statements.push_back($1); }
		| stmts stmt { $1->statements.push_back($2); }
		;
stmt	: var_decl
		| print
		| expr { $$ = new ExprStmtAST(*$1); }
		;
print	: TPRINT expr { $$ = new PrintStmtAST(*$2); }
		;
var_decl: ident ident { $$ = new VariableDeclarationStmtAST(*$1, *$2); }
		| ident ident TASSIGN expr { $$ = new VariableDeclarationStmtAST(*$1, *$2, $4); }
		;
ident	: TIDENTIFIER { $$ = new IdentifierExprAST(*$1); delete $1; }
		;
numeric	: TINTEGER { $$ = new IntegerExprAST(atol($1->c_str())); delete $1; }
		| TDOUBLE { $$ = new DoubleExprAST(atof($1->c_str())); delete $1; }
		;
expr	: ident TASSIGN expr { $$ = new AssignmentExprAST(*$1, *$3); }
		| ident { $$ = $1; }
		| numeric
		| expr b_op expr { $$ = new BinaryOperatorExprAST(*$1, $2, *$3); }
		| TLPAREN expr TRPAREN { $$ = $2; }
		;
b_op	: TADD | TSUB | TMUL | TDIV | TMOD
		;

%%