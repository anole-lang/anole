%{
#include <string>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
%}

delim   [ \t\n]
ws      {delim}+
id      [a-zA-Z_][a-zA-Z0-9_]*
int     [0-9]+
double  {int}\.[0-9]*

%option noyywrap

%%

{ws}                    ;
{id}                    SAVE_TOKEN; return TIDENTIFIER;
{int}                   SAVE_TOKEN; return TINTEGER;
{double}                SAVE_TOKEN; return TDOUBLE;

":"                     return TOKEN(TASSIGN);

.                       printf("Unknown token!\n"); yyterminate();

%%
