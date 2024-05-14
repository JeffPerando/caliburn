
#pragma once

#include <array>
#include <string>

#include "basic.h"
#include "buffer.h"
#include "syntax.h"

namespace caliburn
{
	/*
	CharType might seem redundant compared to TokenType, but you'd be wrong.

	CharType focuses on a subset of 'core' token types, and TokenType is an elaboration on what a token actually means.
	*/
	enum class CharType : uint8_t
	{
		UNKNOWN,
		IDENTIFIER,
		COMMENT,
		WHITESPACE,
		OPERATOR,
		STRING_DELIM,
		INT,
		SPECIAL
	};

	/*
	Tokenizer converts a string into a set of Tokens. See Token in syntax.h

	Note that it currently only supports ASCII. UTF-8 support is planned.
	*/
	struct Tokenizer
	{
	private:
		Buffer<char> buf;
		TextPos pos;

		std::array<CharType, 128> asciiTypes;

		sptr<Token> makeCharTkn(TokenType t)
		{
			return new_sptr<Token>(Token{ doc->text.substr(buf.offset(), 1), t, pos});
		}

	public:
		sptr<TextDoc> doc;

		//TODO use 32-bit wide chars for UTF-8 support
		//or, y'know, find a UTF-8 library (NOT BOOST)
		Tokenizer(sptr<TextDoc> doc);

		virtual ~Tokenizer() = default;

		/*
		Main function for creating tokens.

		Running this multiple times in a row invokes UB. And by that I mean you'll probably get bupkis the second time.
		*/
		std::vector<sptr<Token>> tokenize();

	private:
		/*
		Looks up the char type for the given char.

		This is a function because then logic can easily be inserted.
		*/
		CharType getType(char chr) const;

		/*
		Looks for the fractional and/or exponential component of a floating-point number.

		Return true if it finds one, meaning the literal is a float.
		*/
		bool findFloatFrac();

		/*
		Looks for an int literal.

		Returns the length of the int literal
		*/
		size_t findIntLiteral(out<TokenType> type);

		/*
		Looks for an identifier.

		Returns the length of the identifier
		*/
		size_t findIdentifierLen();

	};

}
