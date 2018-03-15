%{
#include <string>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
%}

delim                   [ \t\n]
ws                      {delim}+
comment                 #[^\n]*
id                      [a-zA-Z_][a-zA-Z0-9_]*
int                     [0-9]+
double                  {int}\.[0-9]*

%option noyywrap

%%

fuck                    return TOKEN(TFUCK);

{ws}                    ;
{comment}               ;

{id}                    SAVE_TOKEN; return TIDENTIFIER;
{int}                   SAVE_TOKEN; return TINTEGER;
{double}                SAVE_TOKEN; return TDOUBLE;

":"                     return TOKEN(TASSIGN);
","                     return TOKEN(TCOMMA);

"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);

"+"                     return TOKEN(TADD);
"-"                     return TOKEN(TSUB);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
"%"                     return TOKEN(TMOD);

"="                     return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);

.                       printf("Unknown token!\n"); yyterminate();

%%
