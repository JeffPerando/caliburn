
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

Tokenizer::Tokenizer(sptr<TextDoc> t) : doc(t), buf(std::vector<char>(t->text.begin(), t->text.end()))
{
	//I'm so sorry for this.

	asciiTypes.fill(CharType::UNKNOWN);

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
		//'a' - 'z'
		auto cap = l + 32;
		asciiTypes[cap] = CharType::IDENTIFIER;
	}

	asciiTypes['_'] = CharType::IDENTIFIER;

	for (auto ch : OPERATOR_CHARS)
	{
		asciiTypes[ch] = CharType::OPERATOR;
	}

	asciiTypes['\''] = CharType::STRING_DELIM;
	asciiTypes['\"'] = CharType::STRING_DELIM;

	asciiTypes['#'] = CharType::COMMENT;

	asciiTypes['('] = CharType::SPECIAL;
	asciiTypes[')'] = CharType::SPECIAL;
	asciiTypes[','] = CharType::SPECIAL;
	asciiTypes['['] = CharType::SPECIAL;
	asciiTypes[']'] = CharType::SPECIAL;
	asciiTypes['{'] = CharType::SPECIAL;
	asciiTypes['}'] = CharType::SPECIAL;
	
	//character 127 (DEL) is a reserved UNKNOWN
	for (auto i = 0; i < 127; ++i)
	{
		if (asciiTypes[i] == CharType::UNKNOWN)
		{
			asciiTypes[i] = CharType::SPECIAL;
		}

	}

}

CharType Tokenizer::getType(char chr) const
{
	if (chr > 127)
	{
		return CharType::IDENTIFIER;
	}

	return asciiTypes[chr];
}

bool Tokenizer::findFloatFrac()
{
	size_t start = buf.offset();

	auto const isInt = LAMBDA_FN(char chr)
	{
		return std::binary_search(DEC_INTS.begin(), DEC_INTS.end(), chr);
	};

	//Find a decimal component
	if (buf.hasRem(2) && buf.cur() == '.' && isInt(buf.peek(1)))
	{
		buf.consume(2);

		while (buf.hasCur() && isInt(buf.cur()))
		{
			buf.consume();
		}

	}

	//Find an exponent
	if (buf.hasRem(2) && std::tolower(buf.cur()) == 'e')
	{
		buf.consume();

		if (auto sign = buf.cur(); sign == '+' || sign == '-')
		{
			buf.consume();
		}

		if (!buf.hasCur() || !isInt(buf.cur()))
		{
			return false;
		}

		while (buf.hasCur() && isInt(buf.cur()))
		{
			buf.consume();
		}

	}

	return buf.offset() > start;
}

size_t Tokenizer::findIntLiteral(out<TokenType> type)
{
	auto validIntChars = &DEC_INTS;

	auto const isInt = LAMBDA(char chr)
	{
		return std::binary_search(validIntChars->begin(), validIntChars->end(), chr);
	};

	auto const first = buf.cur();

	if (!isInt(first))
	{
		return 0;
	}

	size_t startIndex = buf.offset();
	bool isPrefixed = true;
	bool isFloat = false;

	//find either a hex, binary, or octal integer
	if (first == '0' && buf.hasRem(3))
	{
		char litPrefix = std::tolower(buf.peek(1));

		switch (litPrefix)
		{
			case 'b': validIntChars = &BIN_INTS; break;
			case 'c': validIntChars = &OCT_INTS; break;
			case 'x': validIntChars = &HEX_INTS; break;
			default: isPrefixed = false;
		}

		if (isPrefixed)
		{
			buf.consume(2);
		}

	}
	else isPrefixed = false;

	//get the actual numerical digits
	while (buf.hasCur())
	{
		auto digit = buf.cur();
		
		if (isInt(digit) || digit == '_')
		{
			buf.consume();
		}
		else break;

	}

	//find a decimal/float
	if (!isPrefixed)
	{
		isFloat = findFloatFrac();
	}

	if (buf.hasCur())
	{
		auto suffix = std::tolower(buf.cur());
		
		if (suffix == 'f')
		{
			buf.consume();
		}
		else if (suffix == 'd')
		{
			buf.consume();
		}
		else if (suffix == 'u')
		{
			if (buf.hasRem(2) && std::tolower(buf.peek(1)) == 'l')
			{
				buf.consume();
			}

			buf.consume();

		}
		else if (suffix == 'l')
		{
			buf.consume();
		}
		
	}

	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;

	size_t len = (buf.offset() - startIndex);

	//The literal might not be selected
	buf.revertTo(startIndex);

	return len;
}

size_t Tokenizer::findIdentifierLen() const
{
	size_t len = 0;

	for (size_t i = 0; i < buf.remaining(); ++i)
	{
		auto const type = getType(buf.peek(i));

		if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			++len;
		}
		else break;

	}

	return len;
}

std::vector<Token> Tokenizer::tokenize()
{
	std::vector<Token> tokens;

	while (buf.hasCur())
	{
		const size_t start = buf.offset();
		const TextPos startPos = pos;
		auto tknType = TokenType::UNKNOWN;
		size_t tknLen = 0;

		const char current = buf.cur();
		const CharType type = getType(current);

		if (type == CharType::WHITESPACE)
		{
			/*
			See https://en.wikipedia.org/w/index.php?title=Newline
			So there's 2 major line endings we care about:
			Windows \n
			Linux/MacOS \r\n
			Result: We don't care about \r. We just don't. We see it, we skip it. That way line counts are kept sane.
			*/

			buf.consume();

			if (current == '\n')
			{
				pos.newline();
			}
			else if (current != '\r')
			{
				pos.move();
			}

			continue;
		}
		else if (type == CharType::COMMENT)
		{
			buf.consume();

			//skip to the end of the line, which will later invoke the whitespace code
			while (buf.hasCur())
			{
				if (buf.cur() == '\n')
				{
					break;
				}

				if (buf.cur() != '\r')
				{
					pos.move();
				}

				buf.consume();

			}

			//go back to the start
			continue;
		}
		else if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			/*
			Identifiers in Caliburn can start with a number.

			First, we find the lengths of both an identifier and an integer. Whichever is
			longer is the one we go with. If they're the same, the integer is made since it
			should be a valid integer literal, which isn't a valid identifier.
			*/

			const size_t wordLen = findIdentifierLen();
			const size_t intLen = findIntLiteral(tknType);

			if (intLen == 0 && wordLen == 0)
			{
				throw std::runtime_error((std::stringstream() << "Zero width identifier found at " << pos.toStr()).str());
			}

			if (wordLen > intLen)
			{
				tknLen = wordLen;
				tknType = TokenType::IDENTIFIER;

				if (KEYWORDS.count(doc->text.substr(start, tknLen)))
				{
					tknType = TokenType::KEYWORD;
				}

			}
			else
			{
				tknLen = intLen;
			}

		}
		else if (type == CharType::STRING_DELIM)
		{
			if (!buf.hasRem(2))
			{
				throw std::exception("Not enough chars for a string literal");
			}

			//only used for error messaging
			Token delimTkn = makeCharTkn(TokenType::UNKNOWN);

			char delim = current;
			bool foundDelim = false;

			buf.consume();

			while (buf.hasCur())
			{
				char strChar = buf.cur();

				if (strChar == delim)
				{
					buf.consume();
					pos.move();
					foundDelim = true;
					break;
				}

				if (strChar == '\\')
				{
					//Escaped character; skip the backslash and whatever comes after
					//Note: This will be sound when UTF-8 support is added, since UTF literals won't be needed

					buf.consume(2);
					pos.move(2);
					continue;
				}

				if (strChar == '\n')
				{
					buf.consume();
					pos.newline();
					continue;
				}
				
				buf.consume();
				pos.move();

			}

			if (!foundDelim)
			{
				throw std::runtime_error((std::stringstream() << "Unescaped string starts at " << delimTkn.pos.toStr()).str());
			}

			tknType = TokenType::LITERAL_STR;
			tknLen = buf.offset() - start;

		}
		else if (type == CharType::SPECIAL)
		{
			/*
			Special tokens are always 1-char tokens and typically have an override (see TOKEN_TYPE_OVERRIDES)
			*/
			tknLen = 1;

		}
		else if (type == CharType::OPERATOR)
		{
			/*
			* The tokenizer here looks ahead to find the longest possible operator, then will pare back the substring
			* until a valid operator is found.
			*/

			size_t opLen = 1;

			while (buf.hasRem(opLen))
			{
				if (getType(buf.peek(opLen)) != CharType::OPERATOR)
				{
					break;
				}

				++opLen;

			}
			
			while (opLen > 1)
			{
				if (LONG_OPS.count(doc->text.substr(buf.offset(), opLen)))
				{
					tknType = TokenType::OPERATOR;
					tknLen = opLen;
					break;
				}

				--opLen;

			}

			//Couldn't find anything longer, just pass it along as a char token
			if (opLen == 1)
			{
				tknType = TokenType::OPERATOR;
				tknLen = 1;
			}

		}
		else
		{
			//if all else fails, skip it.
			buf.consume();
			pos.move();
			continue;
		}

		auto const content = doc->text.substr(start, tknLen);

		if (auto typeOverride = TOKEN_TYPE_OVERRIDES.find(content); typeOverride != TOKEN_TYPE_OVERRIDES.end())
		{
			tknType = typeOverride->second;
		}

		buf.consume(tknLen);
		pos.move(SCAST<uint32_t>(tknLen));

		tokens.push_back(Token{ content, tknType, startPos });

	}

	return tokens;
}
