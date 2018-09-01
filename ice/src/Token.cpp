#include "Token.h"

#define elif		else if
#define VALUE		( value
#define TOKEN_ID	) token_id


namespace Ice
{
	Token::Token(::std::string value) : value(value)
	{ // just for fun
		if   VALUE == "using"		TOKEN_ID = TOKEN::TUSING;
		elif VALUE == "if"			TOKEN_ID = TOKEN::TIF;
		elif VALUE == "elif"		TOKEN_ID = TOKEN::TELIF;
		elif VALUE == "else"		TOKEN_ID = TOKEN::TELSE;
		elif VALUE == "while"		TOKEN_ID = TOKEN::TWHILE;
		elif VALUE == "do"			TOKEN_ID = TOKEN::TDO;
		elif VALUE == "for"			TOKEN_ID = TOKEN::TFOR;
		elif VALUE == "to"			TOKEN_ID = TOKEN::TTO;
		elif VALUE == "foreach"		TOKEN_ID = TOKEN::TFOREACH;
		elif VALUE == "as"			TOKEN_ID = TOKEN::TAS;
		elif VALUE == "break"		TOKEN_ID = TOKEN::TBREAK;
		elif VALUE == "continue"	TOKEN_ID = TOKEN::TCONTINUE;
		elif VALUE == "return"		TOKEN_ID = TOKEN::TRETURN;
		elif VALUE == "match"		TOKEN_ID = TOKEN::TMATCH;

		elif VALUE == "new"			TOKEN_ID = TOKEN::TNEW;

		elif VALUE == "none"		TOKEN_ID = TOKEN::TNONE;
		elif VALUE == "true"		TOKEN_ID = TOKEN::TTRUE;
		elif VALUE == "false"		TOKEN_ID = TOKEN::TFALSE;

		elif VALUE == "and"			TOKEN_ID = TOKEN::TAND;
		elif VALUE == "or"			TOKEN_ID = TOKEN::TOR;
		elif VALUE == "not"			TOKEN_ID = TOKEN::TNOT;
		else						token_id = TOKEN::TIDENTIFIER;
	}
}

#undef elif
#undef VALUE
#undef TOKEN_ID