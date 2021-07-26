
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
	return isDecInt(chr) || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
}

bool caliburn::isOctInt(char chr)
{
	return chr >= '0' && chr <= '7';
}

bool caliburn::isBinInt(char chr)
{
	return chr == '0' || chr == '1';
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
	size_t tokenLen = 1;
	while (start + tokenLen < txt.length())
	{
		if (!func(txt[start + tokenLen]))
			break;
		tokenLen += 1;
	}
	return tokenLen;
}

size_t caliburn::findStr(std::string& txt, uint64_t cur, char delim)
{
	bool isEscaped = false;
	size_t tokenLen = 1;
	while (txt[cur + tokenLen] != delim && txt[cur + tokenLen - 1] != '\\')
	{
		char current = txt[cur + tokenLen];

		if (current == '\\')
		{
			isEscaped = true;
		}
		//TODO maybe don't include the delimiters in the finished string
		else if (current == delim && !isEscaped)
		{
			break;
		}

		//TODO skip whitespace when an escaped newline is found (multiline string support)
		tokenLen += 1;

	}
	tokenLen += 1;
	return tokenLen;
}

size_t caliburn::scanDecInt(std::string& txt, size_t offset)
{
	size_t count = 0;
	for (size_t i = offset; i < txt.length(); ++i)
	{
		if (!isDecInt(txt[i]))
		{
			break;
		}
		++count;
	}
	return count;
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
	size_t firstInvalid = 0;
	size_t initialLen = 0;
	size_t count = 0;

	if (txt[cur] == '0')
	{
		char litType = txt[cur + 1];
		if (litType == 'x')
		{
			count = 2;
			for (size_t i = cur + 2; i < txt.length(); ++i)
			{
				if (isHexInt(txt[i]) || txt[i] == '_')
				{
					++count;
					continue;
				}

				firstInvalid = i;
				break;
			}
		}
		else if (litType == 'b')
		{
			count = 2;
			for (size_t i = cur + 2; i < txt.length(); ++i)
			{
				if (isBinInt(txt[i]) || txt[i] == '_')
				{
					++count;
					continue;
				}

				firstInvalid = i;
				break;
			}
		}
		else if (litType == 'c')
		{
			count = 2;
			for (size_t i = cur + 2; i < txt.length(); ++i)
			{
				if (isOctInt(txt[i]) || txt[i] == '_')
				{
					++count;
					continue;
				}

				firstInvalid = i;
				break;
			}

		}
		
	}
	
	if (count == 0)
	{
		count = scanDecInt(txt, cur);
		initialLen = count;
		firstInvalid = cur + count;
		if (count == 0)
		{
			return 0;
		}

		//found a float
		if (txt[firstInvalid] == '.' && firstInvalid + 1 < txt.length())
		{
			isFloat = true;
			++count;

			count += scanDecInt(txt, cur + count);
			firstInvalid = cur + count;

			if (firstInvalid + 3 < txt.length())
			{
				char newInvalid = txt[firstInvalid];
				if ((newInvalid == 'e' || newInvalid == 'E') &&
					(txt[firstInvalid + 1] == '+' || txt[firstInvalid + 1] == '-'))
				{
					count += 2;
					count += scanDecInt(txt, cur + count);
					firstInvalid = cur + count;

				}

			}
			
		}

	}
	
	char invalidChr = txt[firstInvalid];
	
	if (invalidChr == 'f' || invalidChr == 'F')
	{
		if (isFloat)
		{
			++count;
		}
		
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

		return count;
	}

	return 0;
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
		else if (txt[cur] == '\"' || txt[cur] == '\'')
		{
			tokenLen = findStr(txt, cur, txt[cur]);
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
		
		}
		
		std::string tokenStr = txt.substr(cur, tokenLen);

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