#include "Token.h"

#define elif else if

namespace Ice
{
	Token::Token(string value) : value(value)
	{
		if (value == "using")
			token_id = TOKEN::TUSING;
		elif (value == "if")
			token_id = TOKEN::TIF;
		elif (value == "else")
			token_id = TOKEN::TELSE;
		elif (value == "while")
			token_id = TOKEN::TWHILE;
		elif (value == "do")
			token_id = TOKEN::TDO;
		elif (value == "for")
			token_id = TOKEN::TFOR;
		elif (value == "to")
			token_id = TOKEN::TTO;
		elif (value == "break")
			token_id = TOKEN::TBREAK;
		elif (value == "continue")
			token_id = TOKEN::TCONTINUE;
		elif (value == "return")
			token_id = TOKEN::TRETURN;
		elif (value == "match")
			token_id = TOKEN::TMATCH;

		elif (value == "new")
			token_id = TOKEN::TNEW;

		elif (value == "none")
			token_id = TOKEN::TNONE;
		elif (value == "true")
			token_id = TOKEN::TTRUE;
		elif (value == "false")
			token_id = TOKEN::TFALSE;

		elif (value == "and")
			token_id = TOKEN::TAND;
		elif (value == "or")
			token_id = TOKEN::TOR;
		elif (value == "not")
			token_id = TOKEN::TNOT;
		else
			token_id = TOKEN::TIDENTIFIER;
	}
}

#undef elif