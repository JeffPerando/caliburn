
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

Tokenizer::Tokenizer(ref<const std::string> str) : doc(str), buf(std::vector<char>(str.begin(), str.end()))
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

	for (auto ch : OPERATOR_CHARS)
	{
		asciiTypes[ch] = CharType::OPERATOR;
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

CharType Tokenizer::getType(char chr) const
{
	return asciiTypes[chr];
}

std::string Tokenizer::findStr(char delim)
{
	std::stringstream ss;
	//copy the start in case we don't find the delimiter
	auto start = pos;
	bool foundDelim = false;

	while (buf.hasNext())
	{
		char current = buf.current();

		if (current == '\\')
		{
			buf.consume(2);
			pos.move(2);
			continue;
		}

		if (current == delim)
		{
			buf.consume();
			pos.move();
			foundDelim = true;
			break;
		}

		if (current == '\n')
		{
			pos.newline();
			doc.startLine(buf.currentIndex() + 1);

			while (buf.hasNext())
			{
				char ws = buf.next();

				if (getType(ws) != CharType::WHITESPACE)
					break;

				if (ws == '\n')
				{
					pos.newline();
					doc.startLine(buf.currentIndex());

				}
				else
				{
					pos.move();
				}

			}

			continue;
		}

		ss << current;
		buf.consume();
		pos.move();

	}

	if (!foundDelim)
	{
		errors->err("Unescaped string", start);
		return "";
	}

	return ss.str();
}

size_t Tokenizer::findIntLiteral(ref<TokenType> type, ref<std::string> lit)
{
	std::vector<char> decInts =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	std::vector<char> hexInts =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' };
	std::vector<char> binInts = { '0', '1' };
	std::vector<char> octInts = { '0', '1', '2', '3', '4', '5', '6', '7' };

	auto validIntChars = &decInts;

	auto isValidInt = lambda(char chr)
	{
		return std::binary_search(validIntChars->begin(), validIntChars->end(), chr);
	};

	char first = buf.current();

	if (!isValidInt(first))
	{
		return 0;
	}

	size_t startIndex = buf.currentIndex();
	bool isPrefixed = true;
	bool isFloat = false;

	std::stringstream ss;

	//find either a hex, binary, or octal integer
	if (first == '0' && buf.remaining() > 2)
	{
		char litType = buf.peek();

		switch (litType)
		{
			case 'x': pass;
			case 'X': validIntChars = &hexInts; break;
			case 'b': pass;
			case 'B': validIntChars = &binInts; break;
			case 'c': pass;
			case 'C': validIntChars = &octInts; break;
			default: isPrefixed = false;
		}

		if (isPrefixed)
		{
			ss << first;
			ss << litType;

			buf.consume(2);

		}

	}
	else isPrefixed = false;

	//get the actual numerical digits
	while (buf.hasNext())
	{
		char current = buf.current();

		if (current == '_')
		{
			buf.consume();
			continue;
		}

		if (isValidInt(current))
		{
			//Make all lowercase hex digits uppercase, for everyone's sanity's sake
			if (current > 96)
				current -= 32;
			ss << current;
			buf.consume();

		}
		else break;

	}

	//find a decimal/float
	if (!isPrefixed)
	{
		//find the fractional component
		if (buf.remaining() >= 2)
		{
			if (buf.current() == '.')
			{
				ss << '.';
				isFloat = true;

				do
				{
					char dec = buf.next();

					if (isValidInt(dec))
					{
						ss << dec;
					}
					else break;

				} while (buf.hasNext());

			}

		}

		//find an exponent
		if (buf.remaining() >= 2)
		{
			char exp = buf.current();

			if (exp == 'e' || exp == 'E')
			{
				ss << exp;

				char sign = buf.next();

				if (sign == '+' || sign == '-')
				{
					ss << sign;
					buf.consume();
				}

				while (buf.hasNext() && isValidInt(buf.current()))
				{
					ss << buf.current();
					buf.consume();
				}

			}

		}

	}

	std::string typeSuffix = "int";
	auto width = 32;

	//find suffix
	if (buf.hasNext())
	{
		char suffix = buf.current();
		buf.consume();

		if (suffix > 96)
			suffix -= 32;

		if (suffix == 'U')
		{
			typeSuffix = "uint";
			suffix = buf.current();
			buf.consume();
		}

		switch (suffix)
		{
			case 'D': width = 64; pass;
			case 'F': isFloat = true; break;
			case 'L': width = 64; break;
			default: buf.rewind();
		}

	}

	//This code is not useless, KEEP IT
	//remember earlier when we checked for a fractional part? Yeah...
	if (isFloat)
		typeSuffix = "float";

	//Attach type information to the end of the literal
	//No, I don't want to expose this syntactically. Those literals look so ugly.
	ss << '_';
	ss << typeSuffix;
	ss << width;

	lit = ss.str();
	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;

	return (buf.currentIndex() - startIndex);
}

size_t Tokenizer::findIdentifierLen(size_t off)
{
	for (size_t i = off; i < buf.length(); ++i)
	{
		auto type = getType(buf[off]);

		if (type != CharType::IDENTIFIER && type != CharType::INT)
		{
			return (i - off);
		}
	}

	return 0;
}

std::vector<sptr<Token>> Tokenizer::tokenize()
{
	std::vector<sptr<Token>> tokens;

	while (buf.hasNext())
	{
		char current = buf.current();

		if (current > 127)
		{
			errors->err("Unidentifiable character", pos);
			buf.consume();
			pos.move();
			continue;
		}

		CharType type = getType(current);

		//See https://en.wikipedia.org/w/index.php?title=Newline

		//So there's 2 major line endings we care about:
		//Windows \n
		//Linux/MacOS \r\n
		//Result: We don't care about \r. We just don't. We see it, we skip it. That way line counts are kept sane.
		if (type == CharType::WHITESPACE)
		{
			if (current == '\r')
			{
				//don't increment pos, just pretend we didn't see it
				buf.consume();
				continue;
			}

			if (significantWhitespace)
			{
				tokens.push_back(new_sptr<Token>(std::string(1, current), TokenType::WHITESPACE, pos, buf.currentIndex(), buf.currentIndex() + 1L));
			}

			buf.consume();

			if (current == '\n')
			{
				pos.newline();
				doc.startLine(buf.currentIndex());
			}
			else if (current == '\t')
			{
				//Yes, we're assuming tabs are 4-wide.
				pos.move(4);
			}
			else
			{
				pos.move();
			}

			continue;
		}

		//Identifiers can start with a number
		if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			auto offset = buf.currentIndex();
			std::string intLit = "";
			TokenType intType = TokenType::UNKNOWN;

			size_t intOffset = findIntLiteral(intType, intLit);
			size_t idOffset = findIdentifierLen(offset);

			//So the idea is simple: We try to make an identifier token, and we try
			//to make an integer token. Whichever is longer is the one we go with. BUT
			//if they're the same length, we go with the integer literal. That way,
			//literals like 0f or 0xDEADBEEF are correctly identified.
			if (idOffset > intOffset)
			{
				auto idStr = doc.text.substr(buf.currentIndex(), idOffset);
				auto idType = TokenType::IDENTIFIER;

				if (std::binary_search(KEYWORDS.begin(), KEYWORDS.end(), idStr))
				{
					auto foundType = STR_TOKEN_TYPES.find(idStr);

					if (foundType == STR_TOKEN_TYPES.end())
					{
						idType = TokenType::KEYWORD;
					}
					else
					{
						idType = foundType->second;
					}

				}

				tokens.push_back(new_sptr<Token>(idStr, idType, pos, buf.currentIndex(), idOffset));

				buf.consume(idOffset);
				pos.move(idOffset);

			}
			else
			{
				tokens.push_back(new_sptr<Token>(intLit, intType, pos, buf.currentIndex(), intOffset));

				buf.consume(intOffset);
				pos.move(intOffset);

			}

			continue;
		}

		if (type == CharType::COMMENT)
		{
			while (buf.hasNext())
			{
				pos.move();

				if (buf.next() == '\n')
					break;

			}

			continue;
		}

		if (type == CharType::STRING_DELIM)
		{
			auto start = buf.currentIndex();
			auto str = findStr(current);

			//findStr should offset col and line
			tokens.push_back(new_sptr<Token>(str, TokenType::LITERAL_STR, pos, start, start - buf.currentIndex()));
			continue;
		}

		if (type == CharType::SPECIAL)
		{
			auto specialType = CHAR_TOKEN_TYPES.find(current);

			if (specialType != CHAR_TOKEN_TYPES.end())
			{
				tokens.push_back(new_sptr<Token>(std::string(1, current), specialType->second, pos, buf.currentIndex(), 1L));
			}

		}
		else if (type == CharType::OPERATOR)
		{
			size_t count;

			for (count = 1; count < buf.remaining(); ++count)
			{
				if (getType(buf.peek(count)) != CharType::OPERATOR)
					break;
			}
			
			size_t opLen = count;

			while (opLen > 1)
			{
				auto testOp = doc.text.substr(buf.currentIndex(), opLen);
				auto meaning = INFIX_OPS.find(testOp);

				if (meaning == INFIX_OPS.end())
				{
					auto specMeaning = SPECIAL_OPS.find(testOp);

					if (specMeaning != SPECIAL_OPS.end())
					{
						tokens.push_back(new_sptr<Token>(testOp, specMeaning->second, pos, buf.currentIndex(), opLen));
						buf.consume(opLen);
						pos.move(opLen);
						break;
					}

					--opLen;
					continue;
				}

				tokens.push_back(new_sptr<Token>(testOp, TokenType::OPERATOR, pos, buf.currentIndex(), opLen));
				buf.consume(opLen);
				pos.move(opLen);
				break;
			}

		}

		//if all else fails, skip it.
		buf.consume();
		pos.move();

	}

	return tokens;
}
