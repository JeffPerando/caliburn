
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

CharType Tokenizer::getType(char chr)
{
	return asciiTypes[chr];
}

std::string Tokenizer::findStr(char delim)
{
	std::stringstream ss;

	while (buf->hasNext())
	{
		char current = buf->currentVal();

		if (current == delim)
		{
			buf->consume();
			++col;
			break;
		}

		if (current == '\n')
		{
			++line;
			col = 1;

			while (buf->hasNext())
			{
				char ws = buf->nextVal();

				if (getType(ws) != CharType::WHITESPACE)
					break;

				if (ws == '\n')
				{
					++line;
					col = 1;
				}
				else
				{
					++col;
				}

			}

			continue;
		}

		ss << current;
		buf->consume();
		++col;

	}

	return ss.str();
}

std::string Tokenizer::findIntLiteral(TokenType& type, uint64_t& offset)
{
	std::vector<char> decInts =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	std::vector<char> hexInts =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' };
	std::vector<char> binInts = { '0', '1' };
	std::vector<char> octInts = { '0', '1', '2', '3', '4', '5', '6', '7' };

	std::vector<char>* validIntChars = &decInts;

	auto isDecInt = [decInts](char chr)
	{
		return std::binary_search(decInts.begin(), decInts.end(), chr);
	};

	auto isValidInt = [validIntChars](char chr)
	{
		return std::binary_search(validIntChars->begin(), validIntChars->end(), chr);
	};

	char first = buf->currentVal();

	if (!isDecInt(first))
	{
		return "";
	}

	size_t startIndex = buf->currentIndex();
	bool isPrefixed = true;
	bool isFloat = false;
	
	std::stringstream ss;

	//find either a hex, binary, or octal integer
	if (first == '0' && buf->remaining() > 2)
	{
		char litType = buf->peekVal();

		switch (litType)
		{
		case 'x':
		case 'X': validIntChars = &hexInts; break;
		case 'b':
		case 'B': validIntChars = &binInts; break;
		case 'c':
		case 'C': validIntChars = &octInts; break;
		default: isPrefixed = false;
		}

		if (isPrefixed)
		{
			ss << first;
			ss << litType;

			buf->consume(2);

		}

	}
	else isPrefixed = false;

	//get the actual numerical digits
	while (buf->hasNext())
	{
		char current = buf->currentVal();
		
		if (current == '_')
		{
			buf->consume();
			continue;
		}

		if (isValidInt(current))
		{
			//Make all lowercase hex digits uppercase, for everyone's sanity's sake
			if (current > 96)
				current -= 32;
			ss << current;
			buf->consume();

		}
		else break;

	}

	//find a decimal/float
	if (!isPrefixed)
	{
		//find the fractional component
		if (buf->remaining() >= 2)
		{
			if (buf->currentVal() == '.')
			{
				ss << '.';
				isFloat = true;

				do
				{
					char dec = buf->nextVal();

					if (isDecInt(dec))
					{
						ss << dec;
					}
					else break;

				} while (buf->hasNext());

			}

		}

		//find an exponent
		if (buf->remaining() >= 2)
		{
			char exp = buf->currentVal();

			if (exp == 'e' || exp == 'E')
			{
				ss << exp;
				
				char sign = buf->nextVal();

				if (sign == '+' || sign == '-')
				{
					ss << sign;
					buf->consume();
				}

				while (buf->hasNext() && isValidInt(buf->currentVal()))
				{
					ss << buf->currentVal();
					buf->consume();
				}

			}

		}

	}

	std::string typeSuffix = "int";
	auto width = 32;

	//find suffix
	if (buf->hasNext())
	{
		char suffix = buf->currentVal();
		buf->consume();

		if (suffix > 96)
			suffix -= 32;
		
		if (suffix == 'U')
		{
			typeSuffix = "uint";
			suffix = buf->currentVal();
			buf->consume();
		}

		switch (suffix)
		{
		case 'D': width = 64;
		case 'F': isFloat = true; break;
		case 'L': width = 64; break;
		default: buf->rewind();
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

	offset = (buf->currentIndex() - startIndex);
	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;
	//we've been using the buffer this whole time to help track this stuff.
	//So we revert it so we don't need to change any code.
	buf->revertTo(startIndex);

	return ss.str();
}

size_t Tokenizer::findIdentifierLen()
{
	size_t offset = 0;

	while (buf->inRange(offset))
	{
		char chrAtOff = buf->peekVal(offset);
		if (getType(chrAtOff) != CharType::IDENTIFIER &&
			getType(chrAtOff) != CharType::INT)
		{
			break;
		}
		++offset;
	}

	return offset;
}

//TODO use 32-bit wide chars for UTF-8 support
//or, y'know, find a UTF-8 library (NOT BOOST)
void Tokenizer::tokenize(std::string& text, std::vector<Token>& tokens)
{
	auto vec = std::vector<char>(text.begin(), text.end());
	auto back = buffer<char>(&vec);
	buf = &back;

	line = 1;
	col = 1;

	while (buf->hasNext())
	{
		char current = buf->currentVal();

		CharType type = getType(current);

		/*
		See https://en.wikipedia.org/w/index.php?title=Newline

		So there's 2 major line endings we care about:
		Windows \n
		Linux/MacOS \r\n
		Result: We don't care about \r. We just don't. We see it, we skip it. That way line counts are kept sane.
		*/
		if (type == CharType::WHITESPACE)
		{
			if (current == '\r')
			{
				//don't increment col, just pretend we didn't see it
				buf->consume();
				continue;
			}

			if (significantWhitespace)
			{
				tokens.push_back(Token(std::string(1, current), TokenType::UNKNOWN, line, col));
			}

			buf->consume();

			if (current == '\n')
			{
				++line;
				col = 1;
			}
			else if (current == '\t')
			{
				//Yes, we're assuming tabs are 4-wide.
				col += 4;
			}
			else
			{
				++col;
			}

			continue;
		}

		if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			std::string intLit = "";
			size_t intOffset = 0;
			TokenType intType = TokenType::UNKNOWN;

			if (type == CharType::INT)
			{
				intLit = findIntLiteral(intType, intOffset);

			}

			size_t idOffset = findIdentifierLen();

			if (idOffset > intOffset)
			{
				auto idStr = text.substr(buf->currentIndex(), idOffset);
				auto idType = TokenType::IDENTIFIER;

				if (std::binary_search(KEYWORDS.begin(), KEYWORDS.end(), idStr))
				{
					auto foundType = strTokenTypes.find(idStr);

					if (foundType == strTokenTypes.end())
					{
						idType = TokenType::KEYWORD;
					}
					else
					{
						idType = foundType->second;
					}

				}

				tokens.push_back(Token(idStr, idType, line, col));

				buf->consume(idOffset);
				col += idOffset;
				
			}
			else
			{
				tokens.push_back(Token(intLit, intType, line, col));

				buf->consume(intOffset);
				col += intOffset;
				
			}

			continue;
		}

		if (type == CharType::COMMENT)
		{
			while (buf->hasNext())
			{
				++col;

				if (buf->nextVal() == '\n')
					break;

			}

			continue;
		}

		if (type == CharType::STRING_DELIM)
		{
			auto str = findStr(current);

			tokens.push_back(Token(str, TokenType::LITERAL_STR, line, col));
			continue;
		}

		if (type == CharType::SPECIAL)
		{
			auto specialType = charTokenTypes.find(current);

			if (specialType != charTokenTypes.end())
			{
				tokens.push_back(Token(std::string(1, current), specialType->second, line, col));
			}
		}
		else if (type == CharType::OPERATOR)
		{
			size_t off;

			for (off = 1; off < buf->remaining(); ++off)
			{
				if (getType(buf->peekVal(off)) != CharType::OPERATOR)
					break;
			}

			auto fullOp = text.substr(buf->currentIndex(), off);
			auto meaning = specialOps.find(fullOp);

			if (meaning != specialOps.end())
			{
				tokens.push_back(Token(fullOp, meaning->second, line, col));
				buf->consume(off);
				col += off;
				continue;
			}

			tokens.push_back(Token(std::string(1, current), TokenType::OPERATOR, line, col));
		}
		
		//if all else fails, skip it.
		buf->consume();
		++col;
		
	}

	buf = nullptr;

}
