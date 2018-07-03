#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>

using std::string;

namespace Ice
{
	class Token
	{
	public:

		enum class TOKEN
		{
			TEND, // #

			TAT, // @
			TATAT, // @@

			TUSING, // using
			TIF,     // if
			TELIF,   // elif
			TELSE,   // else
			TWHILE,  // while
			TDO,	// do
			TFOR,	// for
			TTO,	// to
			TFOREACH, // foreach
			TAS,	// as
			TBREAK,	// break
			TCONTINUE,	// continue
			TRETURN, // return
			TMATCH, // match

			TNEW, // new

			TNONE, // none
			TTRUE, // true
			TFALSE, // false

			TIDENTIFIER, // [a-zA-Z_][a-zA-Z0-9_]*
			TINTEGER,    // [0-9]+
			TDOUBLE,     // [0-9]+\.[0-9]*
			TSTRING,     // "[^"\n]"

			TASSIGN, // :
			TCOMMA,  // ,
			TDOT, // .

			TLPAREN, // (
			TRPAREN, // )
			TLBRACKET, // [
			TRBRACKET, // ]
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
			TNOT, // not !

			TCEQ, // =
			TCNE, // !=
			TCLT, // <
			TCLE, // <=
			TCGT, // >
			TCGE, // >=

			TRET, // =>
		} token_id;
		string value;
		Token(TOKEN token_id, string value) : token_id(token_id), value(value) {}
		Token(TOKEN token_id) : token_id(token_id) {}
		Token(string value);
	};
}

#endif //__TOKEN_H__