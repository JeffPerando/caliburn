
#pragma once

#include <string>
#include <vector>

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
		std::string& txt;
		uint64_t line = 1;
		uint64_t col = 1;
		uint64_t cur = 0;

		CharType asciiTypes[128]{};

	public:
		Tokenizer(std::string& text) : txt(text)
		{
			//I'm so sorry for this.
			for (auto i = 0; i < 128; ++i)
			{
				asciiTypes[i] = CharType::UNKNOWN;
			}

			for (auto i = 0; i <= 32; ++i)
			{
				asciiTypes[i] = CharType::WHITESPACE;
			}

			for (auto i = 0; i < 10; ++i)
			{
				asciiTypes['0' + i] = CharType::INT;
			}

			for (auto i = 0; i < 26; ++i)
			{
				asciiTypes['A' + i] = CharType::IDENTIFIER;
			}

			for (auto i = 0; i < 26; ++i)
			{
				asciiTypes['a' + i] = CharType::IDENTIFIER;
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

			//character 127 (delete) is a reserved UNKNOWN
			for (auto i = 0; i < 127; ++i)
			{
				if (asciiTypes[i] == CharType::UNKNOWN)
				{
					asciiTypes[i] = CharType::SPECIAL;
				}

			}

		}

		virtual ~Tokenizer() {}

		void tokenize(std::vector<Token>& tokens);

	private:
		std::string findStr(char delim);

		std::string findIntLiteral(TokenType& type, uint64_t& offset);

		size_t findIdentifierLen();

	};

}
