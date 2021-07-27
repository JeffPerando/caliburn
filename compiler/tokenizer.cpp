
#include <algorithm>
#include <map>
#include <iostream>

#include "tokenizer.h"

using namespace caliburn;

bool caliburn::isIdentifier(char chr)
{
	return !isComment(chr) && !isStrDelim(chr) && !isWhitespace(chr) &&
		!isOperator(chr) && getSpecial(chr) == TokenType::NONE;
}

bool caliburn::isComment(char chr)
{
	return chr == '#';
}

bool caliburn::isWhitespace(char chr)
{
	return std::string(WHITESPACE).find(chr) != std::string::npos;
}

bool caliburn::isOperator(char chr)
{
	static const auto ops = std::string(OPERATORS);
	return std::binary_search(ops.begin(), ops.end(), chr);
}

bool caliburn::isStrDelim(char chr)
{
	return chr == '\"' || chr == '\'';
}

bool caliburn::isDecInt(char chr)
{
	return chr >= '0' && chr <= '9';
}

bool caliburn::isHexInt(char chr)
{
	return isDecInt(chr) || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F') || chr == '_';
}

bool caliburn::isOctInt(char chr)
{
	return chr >= '0' && chr <= '7' || chr == '_';
}

bool caliburn::isBinInt(char chr)
{
	return chr == '0' || chr == '1' || chr == '_';
}

TokenType caliburn::getSpecial(char chr)
{
	static const std::map<char, TokenType> tokenValues = CALIBURN_TOKEN_VALUES;

	auto found = tokenValues.find(chr);

	if (found == tokenValues.end())
	{
		return TokenType::NONE;
	}

	//I heavily, heavily dislike the standard library
	return found->second;
}

size_t caliburn::find(caliburn::FindFunc func, std::string& txt, uint64_t start)
{
	if (!func)
	{
		return 0;
	}
	size_t tokenLen = 0;
	while (start + tokenLen < txt.length())
	{
		if (!func(txt[start + tokenLen]))
			break;
		tokenLen += 1;
	}
	return tokenLen;
}

//TODO fix memory leak (needs proper memory allocator support)
char* caliburn::findStr(std::string& txt, uint64_t& cur, uint64_t& line)
{
	constexpr size_t bufferLen = 4096;
	char* buffer = new char[bufferLen]{0};
	bool isEscaped = false;
	size_t tokenLen = 0;
	size_t i;

	char delim = txt[cur];

	for (i = cur + 1; i < txt.length(); ++i)
	{
		if (tokenLen == bufferLen)
		{
			if (txt[i] != delim)
			{
				//TODO complain about size of string literal
			}
			break;
		}

		char current = txt[i];

		if (current == '\\')
		{
			isEscaped = true;
		}
		else if (current == '\n')
		{
			//skip whitespace, keep track of line #
			do
			{
				i += 1;

				if (i == txt.length())
				{
					break;
				}

				if (txt[i] == '\n')
				{
					line += 1;
				}

			} while (isWhitespace(txt[i]));

			continue;
		}
		else if (current == delim && !isEscaped)
		{
			break;
		}

		buffer[tokenLen] = current;
		tokenLen += 1;
		
	}

	cur = i;
	
	return buffer;
}

size_t caliburn::findIntLiteral(std::string& txt, uint64_t cur, TokenType& type)
{
	//integer validation
	//the alternative was writing regular expressions; which would be fine, but not performant.
	if (!isDecInt(txt[cur]))
	{
		return 0;
	}

	bool isValidLit = true;
	bool isLong = false;
	bool isFloat = false;
	bool isDouble = false;
	size_t firstInvalid = cur + 1;
	size_t count = 0;

	//find either a hex, binary, or octal integer
	if (txt[cur] == '0' && cur + 1 < txt.length())
	{
		char litType = txt[cur + 1];
		FindFunc func = nullptr;

		if (litType == 'x')
		{
			func = isHexInt;
		}
		else if (litType == 'b')
		{
			func = isBinInt;
		}
		else if (litType == 'c')
		{
			func = isOctInt;
		}

		if (func)
		{
			count = 2 + find(func, txt, cur + 2);
			firstInvalid = count + 1;

			if (count == 2)
			{
				isValidLit = false;
			}

		}
		
	}
	
	//find a base-10 int
	if (count == 0)
	{
		//will be at least one, given the check at the start of the function
		count = find(&isDecInt, txt, cur);
		firstInvalid = cur + count;
		
		//find a float
		if (firstInvalid + 1 < txt.length())
		{
			if (txt[firstInvalid] == '.')
			{
				isFloat = true;
				++count;

				size_t fracCount = find(&isDecInt, txt, firstInvalid + 1);
				count += fracCount;
				firstInvalid = cur + count;

				if (fracCount == 0)
				{
					isValidLit = false;
				}

			}
			
		}

		//find an exponent
		if (firstInvalid + 3 < txt.length())
		{
			if ((txt[firstInvalid] == 'e' || txt[firstInvalid] == 'E') &&
				(txt[firstInvalid + 1] == '+' || txt[firstInvalid + 1] == '-'))
			{
				count += 2;
				firstInvalid += 2;
				size_t expCount = find(&isDecInt, txt, firstInvalid);
				count += expCount;
				firstInvalid += expCount;

				if (expCount == 0)
				{
					isValidLit = false;
				}

			}

		}

	}
	
	//find suffix
	if (firstInvalid < txt.length())
	{
		char invalidChr = txt[firstInvalid];

		if (invalidChr == 'f' || invalidChr == 'F')
		{
			if (!isFloat)
			{
				isFloat = true;
			}

			++count;

		}
		else if (invalidChr == 'd' || invalidChr == 'D')
		{
			if (isFloat)
			{
				isDouble = true;
				++count;
			}

		}
		else if (invalidChr == 'l' || invalidChr == 'L')
		{
			if (!isFloat)
			{
				isLong = true;
				++count;
			}

		}

	}
	
	if (isValidLit)
	{
		if (isDouble)
		{
			type = TokenType::LITERAL_DOUBLE;
		}
		else if (isFloat)
		{
			type = TokenType::LITERAL_FLOAT;
		}
		else if (isLong)
		{
			type = TokenType::LITERAL_LONG;
		}
		else
		{
			type = TokenType::LITERAL_INT;
		}

	}
	else
	{
		type = TokenType::LITERAL_INVALID;
	}

	return count;
}

//TODO use 32-bit wide chars for UTF-8 support
//or, y'know, find a UTF-8 library (NOT BOOST)
void caliburn::tokenize(std::string& txt, std::vector<Token>& tokens)
{
	uint64_t line = 1, col = 1;
	uint64_t cur = 0;

	std::map<std::string, TokenType> specialOps = CALIBURN_SPECIAL_OPERATORS;

	while (cur < txt.length())
	{
		int hashtags = 0;
		bool isComment = false;
		bool isMultilineComment = false;
		
		//Avoid comments/preprocessor directives
		if (txt[cur] == '#')
		{
			isComment = true;
			hashtags++;

			//avoid multiline comments
			if (hashtags == 3)
			{
				hashtags = 0;
				isMultilineComment = !isMultilineComment;

				if (!isMultilineComment)
				{
					isComment = false;

				}

			}

			cur += 1;
			col += 1;

			continue;
		}
		else
		{
			hashtags = 0;
		}

		if (isComment)
		{
			cur += 1;
			col += 1;
			continue;
		}

		//Avoid whitespace
		if (isWhitespace(txt[cur]))
		{
			if (txt[cur] == '\n')
			{
				line += 1;
				col = 1;

				if (!isMultilineComment)
				{
					isComment = false;

				}

			}
			else
			{
				col += 1;

			}

			cur += 1;
			continue;
		}

		uint64_t tokenLen = 1;
		const char* tokenPtr = nullptr;
		TokenType tokenID = getSpecial(txt[cur]);

		if (tokenID != TokenType::NONE)
		{
			tokens.push_back(Token(std::string(1, txt[cur]), tokenID, line, col));
			cur += 1;
			col += 1;
			continue;
		}
		
		if (isOperator(txt[cur]))
		{
			tokenLen = find(&caliburn::isOperator, txt, cur);
			tokenID = TokenType::OPERATOR;
		}
		else if (isStrDelim(txt[cur]))
		{
			tokenPtr = findStr(txt, cur, line);
			tokenID = TokenType::LITERAL_STR;
		}
		else
		{
			size_t intLen = findIntLiteral(txt, cur, tokenID);
			size_t identLen = find(&caliburn::isIdentifier, txt, cur);
			
			if (identLen > intLen)
			{
				tokenLen = identLen;
				tokenID = TokenType::IDENTIFIER;
			}
			else
			{
				tokenLen = intLen;
			}

		}
		
		std::string tokenStr;
		
		if (tokenPtr != nullptr)
		{
			tokenStr = std::string(tokenPtr, tokenLen);
		}
		else
		{
			tokenStr = txt.substr(cur, tokenLen);
		}
		
		if (tokenID == TokenType::IDENTIFIER)
		{
			if (std::binary_search(KEYWORDS.begin(), KEYWORDS.end(), tokenStr))
			{
				tokenID = TokenType::KEYWORD;

				if (tokenStr == "true" || tokenStr == "false")
				{
					tokenID = TokenType::LITERAL_BOOL;
				}

			}

		}
		else if (tokenID == TokenType::OPERATOR)
		{
			auto found = specialOps.find(tokenStr);
			if (found != specialOps.end())
			{
				tokenID = found->second;
			}
		}

		tokens.push_back(Token(tokenStr, tokenID, line, col));
		cur += tokenLen;
		col += tokenLen;

	}

}