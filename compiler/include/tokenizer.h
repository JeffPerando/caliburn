
#pragma once

#include <array>
#include <string>

#include "basic.h"
#include "buffer.h"
#include "error.h"
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

		/*
		This exists... for some reason.
		I couldn't tell you why though. It completely breaks the parser.
		*/
		bool significantWhitespace = false;

		std::array<CharType, 128> asciiTypes;

		sptr<Token> makeCharTkn(TokenType t)
		{
			return new_sptr<Token>(buf.cur(), t, pos, buf.currentIndex());
		}

	public:
		const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::TOKENIZER);

		sptr<TextDoc> doc;

		//TODO use 32-bit wide chars for UTF-8 support
		//or, y'know, find a UTF-8 library (NOT BOOST)
		Tokenizer(sptr<TextDoc> doc);

		virtual ~Tokenizer() {}

		/*
		Main function for creating tokens.

		Running this multiple times in a row invokes UB. And by that I mean you'll probably get bupkis the second time.
		*/
		std::vector<sptr<Token>> tokenize();

		void enableSignificantWhitespace()
		{
			significantWhitespace = true;
		}

	private:
		/*
		Looks up the char type for the given char.

		This is a function because then logic can easily be inserted.
		*/
		CharType getType(char chr) const;

		/*
		Looks for and returns a string literal. Assumes it starts on a valid delimiter
		*/
		std::string findStr();

		/*
		Looks for an int literal.

		Returns the length of the int literal
		*/
		size_t findIntLiteral(ref<TokenType> type, ref<std::string> offset);

		/*
		Looks for an identifier.

		Returns the length of the identifier
		*/
		size_t findIdentifierLen();

	};

}
