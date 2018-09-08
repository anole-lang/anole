#ifndef __TOKEN_H__
#define __TOKEN_H__


namespace Ice
{
	class Token
	{
	public:

		enum class TOKEN
		{
			TEND,       // #

			TAT,        // @
			TATAT,      // @@

			TUSING,     // using
			TIF,        // if
			TELIF,      // elif
			TELSE,      // else
			TWHILE,     // while
			TDO,	    // do
			TFOR,	    // for
			TTO,	    // to
			TFOREACH,   // foreach
			TAS,	    // as
			TBREAK,	    // break
			TCONTINUE,	// continue
			TRETURN,    // return
			TMATCH,     // match

			TNEW,       // new

			TNONE,      // none
			TTRUE,      // true
			TFALSE,     // false

			TIDENTIFIER, // [a-zA-Z_][a-zA-Z0-9_]*
			TINTEGER,    // [0-9]+
			TDOUBLE,     // [0-9]+\.[0-9]*
			TSTRING,     // "[^"\n]"

			TASSIGN,    // :
			TCOMMA,     // ,
			TDOT,       // .

			TLPAREN,    // (
			TRPAREN,    // )
			TLBRACKET,  // [
			TRBRACKET,  // ]
			TLBRACE,    // {
			TRBRACE,    // }

			TADD,       // +
			TSUB,       // -
			TMUL,       // *
			TDIV,       // /
			TMOD,       // %

			TBAND,      // &
			TBOR,       // |
			TBXOR,      // ^
			TBNEG,      // ~
			TBLS,       // <<
			TBRS,       // >>

			TAND,       // and
			TOR,        // or
			TNOT,       // not !

			TCEQ,       // =
			TCNE,       // !=
			TCLT,       // <
			TCLE,       // <=
			TCGT,       // >
			TCGE,       // >=

			TRET        // =>
		} token_id;

		::std::string value;

		Token(TOKEN token_id, ::std::string value) : token_id(token_id), value(value) {}
		Token(TOKEN token_id) : token_id(token_id) {}
		Token(::std::string value);
	};

    using TOKEN = Token::TOKEN;
}

#endif //__TOKEN_H__