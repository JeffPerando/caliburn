
#pragma once

#include <array>
#include <string>

#include "basic.h"
#include "buffer.h"
#include "error.h"
#include "syntax.h"

namespace caliburn
{
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

	struct Tokenizer
	{
	private:
		Buffer<char> buf;
		TextPos pos;

		bool significantWhitespace = false;

		std::array<CharType, 128> asciiTypes;

		sptr<Token> makeCharTkn(TokenType t)
		{
			return new_sptr<Token>(buf.current(), t, pos, buf.currentIndex());
		}

	public:
		const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::TOKENIZER);

		sptr<TextDoc> doc;

		//TODO use 32-bit wide chars for UTF-8 support
		//or, y'know, find a UTF-8 library (NOT BOOST)
		Tokenizer(sptr<TextDoc> doc);

		virtual ~Tokenizer() {}

		std::vector<sptr<Token>> tokenize();

		void enableSignificantWhitespace()
		{
			significantWhitespace = true;
		}

	private:
		CharType getType(char chr) const;

		std::string findStr();

		size_t findIntLiteral(ref<TokenType> type, ref<std::string> offset);

		size_t findIdentifierLen();

	};

}
