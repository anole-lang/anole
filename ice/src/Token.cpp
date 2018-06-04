#include "Token.h"

#define elif		else if
#define VALUE		(value
#define equals		==
#define assign		)token_id = 
#define to			
#define TOKEN_ID	

namespace Ice
{
	Token::Token(string value) : value(value)
	{ // just for fun
		if   VALUE equals "using"		assign TOKEN::TUSING	to TOKEN_ID;
		elif VALUE equals "if"			assign TOKEN::TIF		to TOKEN_ID;
		elif VALUE equals "else"		assign TOKEN::TELSE		to TOKEN_ID;
		elif VALUE equals "while"		assign TOKEN::TWHILE	to TOKEN_ID;
		elif VALUE equals "do"			assign TOKEN::TDO		to TOKEN_ID;
		elif VALUE equals "for"			assign TOKEN::TFOR		to TOKEN_ID;
		elif VALUE equals "to"			assign TOKEN::TTO		to TOKEN_ID;
		elif VALUE equals "break"		assign TOKEN::TBREAK	to TOKEN_ID;
		elif VALUE equals "continue"	assign TOKEN::TCONTINUE to TOKEN_ID;
		elif VALUE equals "return"		assign TOKEN::TRETURN	to TOKEN_ID;
		elif VALUE equals "match"		assign TOKEN::TMATCH	to TOKEN_ID;

		elif VALUE equals "new"			assign TOKEN::TNEW		to TOKEN_ID;

		elif VALUE equals "none"		assign TOKEN::TNONE		to TOKEN_ID;
		elif VALUE equals "true"		assign TOKEN::TTRUE		to TOKEN_ID;
		elif VALUE equals "false"		assign TOKEN::TFALSE	to TOKEN_ID;

		elif VALUE equals "and"			assign TOKEN::TAND		to TOKEN_ID;
		elif VALUE equals "or"			assign TOKEN::TOR		to TOKEN_ID;
		elif VALUE equals "not"			assign TOKEN::TNOT		to TOKEN_ID;
		else							token_id = TOKEN::TIDENTIFIER;
	}
}

#undef elif
#undef VALUE
#undef equals
#undef assign
#undef to
#undef TOKEN_ID