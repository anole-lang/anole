#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

namespace Ice
{
	class Token
	{
	public:
		enum class TOKEN
		{
			TEND, // #

			TAT, // @

			TIF,     // if
			TELSE,   // else
			TWHILE,  // while
			TFOR,	// for
			TTO,	// to
			TRETURN, // return

			TIDENTIFIER, // [a-zA-Z_][a-zA-Z0-9_]*
			TINTEGER,    // [0-9]+
			TDOUBLE,     // [0-9]+\.[0-9]*
			TSTRING,     // "[^"\n]"

			TASSIGN, // :
			TCOMMA,  // ,

			TESCAPE, // "\"

			TLPAREN, // (
			TRPAREN, // )
			TLBRACE, // {
			TRBRACE, // }

			TADD, // +
			TSUB, // -
			TMUL, // *
			TDIV, // /
			TMOD, // %

			TBAND, // &
			TBOR,  // |
			TBXOR, // ^
			TBNEG, // ~
			TBLS,  // <<
			TBRS,  // >>

			TAND, // and
			TOR,  // or
			TNOT, // not

			TCEQ, // =
			TCNE, // !=
			TCLT, // <
			TCLE, // <=
			TCGT, // >
			TCGE  // >=
		} token_id;
		std::string value;
		Token(TOKEN token_id, std::string value) : token_id(token_id), value(value) {}
		Token(TOKEN token_id) : token_id(token_id) {}
		Token(std::string value);
	};
}

#endif //__TOKEN_H__