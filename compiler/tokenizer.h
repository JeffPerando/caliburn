
#pragma once

#include <string>

#include "buffer.h"
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

	class Tokenizer
	{
		buffer<char>* buf = nullptr;
		uint64_t line = 1;
		uint64_t col = 1;

		bool significantWhitespace = false;

		CharType asciiTypes[128]{};

	public:
		Tokenizer()
		{
			//I'm so sorry for this.

			//Initialize array; Can't find an idiomatic C/C++ way to do this without memset.
			for (auto i = 0; i < 128; ++i)
			{
				asciiTypes[i] = CharType::UNKNOWN;
			}

			for (auto i = 0; i <= 32; ++i)
			{
				asciiTypes[i] = CharType::WHITESPACE;
			}

			for (auto c = '0'; c <= '9'; ++c)
			{
				asciiTypes[c] = CharType::INT;
			}

			for (auto l = 'A'; l <= 'Z'; ++l)
			{
				asciiTypes[l] = CharType::IDENTIFIER;
				asciiTypes[l + 32] = CharType::IDENTIFIER;
			}

			for (auto i = 0; i < OPERATORS.length(); ++i)
			{
				asciiTypes[OPERATORS[i]] = CharType::OPERATOR;

			}

			asciiTypes['#'] = CharType::COMMENT;
			asciiTypes[','] = CharType::SPECIAL;
			asciiTypes['('] = CharType::SPECIAL;
			asciiTypes[')'] = CharType::SPECIAL;
			asciiTypes['{'] = CharType::SPECIAL;
			asciiTypes['}'] = CharType::SPECIAL;
			asciiTypes['['] = CharType::SPECIAL;
			asciiTypes[']'] = CharType::SPECIAL;
			asciiTypes['\''] = CharType::STRING_DELIM;
			asciiTypes['\"'] = CharType::STRING_DELIM;

			//character 127 (DEL) is a reserved UNKNOWN
			for (auto i = 0; i < 127; ++i)
			{
				if (asciiTypes[i] == CharType::UNKNOWN)
				{
					asciiTypes[i] = CharType::SPECIAL;
				}

			}

		}

		virtual ~Tokenizer() {}

		void tokenize(std::string& text, std::vector<Token>& tokens);

		Tokenizer* enableSignificantWhitespace()
		{
			significantWhitespace = true;
			return this;
		}

	private:
		CharType getType(char chr);

		std::string findStr(char delim);

		std::string findIntLiteral(TokenType& type, uint64_t& offset);

		size_t findIdentifierLen();

	};

}
