
#include <algorithm>
#include <map>
#include <iostream>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

std::string Tokenizer::findStr(char delim)
{
	std::stringstream ss;

	while (cur < txt.length())
	{
		char current = txt[cur];

		if (current == delim)
		{
			++col;
			++cur;
			break;
		}

		if (current == '\n')
		{
			++line;
			col = 0;

			while (cur < txt.length())
			{
				char ws = txt[cur];

				if (asciiTypes[ws] != CharType::WHITESPACE)
					break;

				if (ws == '\n')
				{
					++line;
					col = 0;
				}

				++col;
				++cur;

			}

			continue;
		}

		ss << current;
		++col;
		++cur;

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

	if (!std::binary_search(decInts.begin(), decInts.end(), txt[cur]))
	{
		return "";
	}

	bool isPrefixed = true;
	bool isFloat = true;
	int width = 32;

	std::vector<char>* validIntChars = &decInts;

	//find either a hex, binary, or octal integer
	if (txt[cur] == '0' && cur + 2 < txt.length())
	{
		char litType = txt[cur + 1];

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

	}

	std::stringstream ss;

	size_t offset = 0;
	size_t firstInvalid = cur + 1;

	if (isPrefixed)
	{
		ss << txt[cur];
		ss << txt[cur + 1];
		++firstInvalid;
	}

	while (firstInvalid < txt.length())
	{
		++firstInvalid;

		char current = txt[firstInvalid];

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
		//find a float
		if (firstInvalid + 2 < txt.length())
		{
			if (txt[firstInvalid] == '.')
			{
				isFloat = true;

				do
				{
					ss << txt[firstInvalid];
					++firstInvalid;

				} while (firstInvalid < txt.length() && std::binary_search(validIntChars->begin(), validIntChars->end(), txt[firstInvalid]));

			}

		}

		//find an exponent
		if (firstInvalid + 2 < txt.length())
		{
			if (txt[firstInvalid] == 'e' || txt[firstInvalid] == 'E')
			{
				ss << txt[firstInvalid];
				++firstInvalid;

				if (txt[firstInvalid] == '+' || txt[firstInvalid] == '-')
				{
					ss << txt[firstInvalid];
				}

				while (firstInvalid < txt.length() && std::binary_search(validIntChars->begin(), validIntChars->end(), txt[firstInvalid]))
				{
					ss << txt[firstInvalid];
					++firstInvalid;
				}

			}

		}

	}

	//find suffix
	if (firstInvalid + 1 < txt.length())
	{
		char invalidChr = txt[firstInvalid];

		if (invalidChr == 'f' || invalidChr == 'F')
		{
			isFloat = true;
		}
		else if (invalidChr == 'd' || invalidChr == 'D')
		{
			isFloat = true;
			width = 64;
		}
		else if (invalidChr == 'l' || invalidChr == 'L')
		{
			width = 64;
		}

	}

	ss << '_';
	ss << isFloat ? "float" : "int";
	ss << width;

	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;
	offset = (firstInvalid - cur);

	return ss.str();
}

size_t Tokenizer::findIdentifierLen()
{
	size_t offset = 0;

	while (cur + offset < txt.length() && (asciiTypes[txt[cur + offset]] == CharType::IDENTIFIER || asciiTypes[txt[cur + offset]] == CharType::INT))
	{
		++offset;
	}

	return offset;
}

//TODO use 32-bit wide chars for UTF-8 support
//or, y'know, find a UTF-8 library (NOT BOOST)
void Tokenizer::tokenize(std::vector<Token>& tokens)
{
	line = 1;
	col = 1;
	cur = 0;

	while (cur < txt.length())
	{
		if (txt[cur] == '\n')
		{
			++line;
			col = 1;
			++cur;

			continue;
		}

		CharType type = asciiTypes[txt[cur]];

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
				auto idStr = txt.substr(cur, idOffset);
				auto idType = TokenType::IDENTIFIER;

				auto foundType = strTokenTypes.find(idStr);
				if (foundType != strTokenTypes.end())
				{
					idType = foundType->second;
				}

				tokens.push_back(Token(idStr, idType, line, col));

				col += idOffset;
				cur += idOffset;

			}
			else
			{
				tokens.push_back(Token(intLit, intType, line, col));

				col += intOffset;
				cur += intOffset;

			}

			continue;
		}
		else if (type == CharType::SPECIAL)
		{
			auto specialType = charTokenTypes.find(txt[cur]);

			if (specialType != charTokenTypes.end())
			{
				tokens.push_back(Token(std::string(1, txt[cur]), specialType->second, line, col));
			}
		}
		else if (type == CharType::OPERATOR)
		{
			tokens.push_back(Token(std::string(1, txt[cur]), TokenType::OPERATOR, line, col));
		}
		else if (type == CharType::COMMENT)
		{
			do
			{
				++cur;
				++col;
			}
			while (txt[cur] != '\n' && cur < txt.length());

			continue;
		}
		else if (type == CharType::STRING_DELIM)
		{
			auto str = findStr(txt[cur]);

			tokens.push_back(Token(str, TokenType::LITERAL_STR, line, col));
			continue;
		}
		
		//if all else fails, skip it.
		++col;
		++cur;
		
	}

}
