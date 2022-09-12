
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

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

				if (asciiTypes[ws] != CharType::WHITESPACE)
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

	char start = buf->currentVal();

	if (!std::binary_search(decInts.begin(), decInts.end(), start))
	{
		return "";
	}

	size_t off = buf->currentIndex();
	bool isPrefixed = true;
	bool isFloat = true;
	int width = 32;

	std::vector<char>* validIntChars = &decInts;

	std::stringstream ss;

	//find either a hex, binary, or octal integer
	if (start == '0' && buf->remaining() > 2)
	{
		char litType = buf->peekVal();

		if (litType == 'x')
		{
			validIntChars = &hexInts;
		}
		else if (litType == 'b')
		{
			validIntChars = &binInts;
		}
		else if (litType == 'c')
		{
			validIntChars = &octInts;
		}
		else
		{
			isPrefixed = false;
		}

		if (isPrefixed)
		{
			ss << start;
			ss << litType;

			buf->consume();

		}

	}

	while (buf->hasNext())
	{
		char current = buf->nextVal();

		if (current == '_') continue;

		if (std::binary_search(validIntChars->begin(), validIntChars->end(), current))
		{
			ss << current;
		}
		else
		{
			break;
		}
	}

	if (!isPrefixed)
	{
		//find a decimal/float
		if (buf->remaining() >= 2)
		{
			if (buf->currentVal() == '.')
			{
				ss << '.';
				isFloat = true;

				while (buf->hasNext())
				{
					char dec = buf->nextVal();

					if (std::binary_search(validIntChars->begin(), validIntChars->end(), dec))
					{
						ss << dec;
					}
					else
					{
						break;
					}

				}

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

				while (buf->hasNext() && std::binary_search(validIntChars->begin(), validIntChars->end(), buf->currentVal()))
				{
					ss << buf->currentVal();
					buf->consume();
				}

			}

		}

	}

	//find suffix
	if (buf->hasNext())
	{
		char suffix = buf->currentVal();
		buf->consume();

		if (suffix == 'f' || suffix == 'F')
		{
			isFloat = true;
		}
		else if (suffix == 'd' || suffix == 'D')
		{
			isFloat = true;
			width = 64;
		}
		else if (suffix == 'l' || suffix == 'L')
		{
			width = 64;
		}
		else
		{
			buf->rewind();
		}

	}

	//Attach type information to the end of the literal
	//No, I don't want to expose this syntactically. Those literal look so ugly.
	ss << '_';
	ss << isFloat ? "float" : "int";
	ss << width;

	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;
	offset = (buf->currentIndex() - off);
	//we've been using the buffer this whole time to help track this stuff.
	//So we revert it so we don't need to change any code.
	buf->revertTo(off);

	return ss.str();
}

size_t Tokenizer::findIdentifierLen()
{
	size_t offset = 0;

	while (buf->inRange(offset))
	{
		char chrAtOff = buf->peekVal(offset);
		if (asciiTypes[chrAtOff] != CharType::IDENTIFIER &&
			asciiTypes[chrAtOff] != CharType::INT)
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

		if (current == '\n')
		{
			buf->consume();

			++line;
			col = 1;

			continue;
		}

		CharType type = asciiTypes[current];

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

				auto foundType = strTokenTypes.find(idStr);
				if (foundType != strTokenTypes.end())
				{
					idType = foundType->second;
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
				if (asciiTypes[buf->peekVal(off)] != CharType::OPERATOR)
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
