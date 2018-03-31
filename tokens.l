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

%option noyywrap

%%

@                       return TOKEN(TAT);

if                      return TOKEN(TIF);
else                    return TOKEN(TELSE);
while                   return TOKEN(TWHILE);

return                  return TOKEN(TRETURN);

{ws}                    ;
{comment}               ;

{id}                    SAVE_TOKEN; return TIDENTIFIER;
{int}                   SAVE_TOKEN; return TINTEGER;

":"                     return TOKEN(TASSIGN);
";"                     return ';';
","                     return TOKEN(TCOMMA);

"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                    return TOKEN(TLBRACE);
"}"                    return TOKEN(TRBRACE);

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
