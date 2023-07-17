
#pragma once

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
		std::string text;
		Buffer<char> buf;
		TextPos pos;

		bool significantWhitespace = false;

		CharType asciiTypes[128]{};

	public:
		const uptr<ErrorHandler> errors = new_uptr<ErrorHandler>(CompileStage::TOKENIZER);

		TextDoc doc;

		//TODO use 32-bit wide chars for UTF-8 support
		//or, y'know, find a UTF-8 library (NOT BOOST)
		Tokenizer(ref<std::string> str);

		virtual ~Tokenizer() {}

		std::vector<sptr<Token>> tokenize();

		void enableSignificantWhitespace()
		{
			significantWhitespace = true;
		}

	private:
		CharType getType(char chr) const;

		std::string findStr(char delim);

		std::string findIntLiteral(ref<TokenType> type, ref<uint64_t> offset);

		size_t findIdentifierLen();

	};

}
