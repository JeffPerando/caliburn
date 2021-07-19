
#include <map>

#include "tokenizer.h"

size_t caliburn::findOp(std::string txt, uint64_t cur)
{
	size_t tokenLen = 1;
	while (isOperator(txt[cur + tokenLen]))
	{
		tokenLen += 1;
	}
	return tokenLen;
}

size_t caliburn::findStr(std::string txt, uint64_t cur, char delim)
{
	size_t tokenLen = 1;
	while (txt[cur + tokenLen] != delim && txt[cur + tokenLen - 1] != '\\')
	{
		//TODO throw exception if a newline is found
		tokenLen += 1;
	}
	tokenLen += 1;
	return tokenLen;
}

size_t caliburn::findInt(std::string txt, uint64_t cur)
{
	size_t tokenLen = 1;
	while (isInt(txt[cur + tokenLen]))
	{
		tokenLen += 1;
	}
	return tokenLen;
}

size_t caliburn::findIdentifier(std::string txt, uint64_t cur)
{
	size_t tokenLen = 1;
	while (isIdentifier(txt[cur + tokenLen]))
	{
		tokenLen += 1;
	}
	return tokenLen;
}

bool caliburn::isIntStart(char chr)
{
	return chr >= '0' && chr <= '9';
}

bool caliburn::isInt(char chr)
{
	return isIntStart(chr) || std::string(".f_xE+-").find(chr) > 0 || (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

bool caliburn::isIdentifier(char chr)
{
	return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z') || isIntStart(chr) || (chr == '_');
}

bool caliburn::isOperator(char chr)
{
	return std::string("=+-*/<>%^&|$!~").find(chr) > 0;
}

//TODO use 32-bit wide chars for dat UTF-8 support
//or, y'know, find a UTF-8 library (NOT BOOST)
void caliburn::tokenize(std::string txt, std::vector<Token>& tokens)
{
	uint64_t line = 1, col = 1;
	uint64_t cur = 0;

	//TODO use std string and uint64_t
	std::map<const char*, int> tokenValues = CALIBURN_TOKEN_VALUES;

	for (auto kw : CALIBURN_KEYWORDS)
	{
		tokenValues[kw] = CALIBURN_T_KEYWORD;
	}

	while (cur < txt.length())
	{
		//Avoid whitespace
		if (CALIBURN_WHITESPACE.find(txt[cur]))
		{
			if (txt[cur] == '\n')
			{
				line += 1;
				col = 1;
			}
			else
			{
				col += 1;
			}
			cur += 1;
			continue;
		}

		//Avoid comments
		if (txt[cur] == '#')
		{
			if (txt[cur + 1] == '#' && txt[cur + 1] == txt[cur + 2])
			{
				//avoid multiline comments
				cur += 3;
				while (txt.substr(cur, 3) != "###")
				{
					if (txt[cur] == '\n')
					{
						line += 1;
						col = 1;
					}
					cur += 1;
					col += 1;
				}
				cur += 3;
				col += 3;
			}
			else
			{
				//avoid single line comments
				while (txt[cur] != '\n')
				{
					cur += 1;
				}
				cur += 1;
				line += 1;
				col = 1;
			}

			continue;
		}

		uint64_t tokenID = 0;
		auto tokenItr = tokenValues.find(&txt[cur]);

		if (tokenItr != tokenValues.end() && (!isOperator(txt[cur]) || !isOperator(txt[cur + 1])))
		{
			tokens.push_back(Token(std::string(1, txt[cur]), tokenID, line, col));
			cur += 1;
			col += 1;
			continue;
		}

		size_t tokenLen = 1;

		if (isIntStart(txt[cur]) && (isInt(txt[cur + 1]) || !isIdentifier(txt[cur + 1])))
		{
			tokenLen = findInt(txt, cur);
			if (isIdentifier(txt[cur + tokenLen + 1]))
			{
				tokenLen = findIdentifier(txt, cur);
				tokenID = CALIBURN_T_IDENTIFIER;

			}
			else
			{
				//TODO improve logic, figure out ahead of time if literal is a float or a long or what
				tokenID = CALIBURN_T_LITERAL_INT;
			}
		}
		else if (isIdentifier(txt[cur]))
		{
			tokenLen = findIdentifier(txt, cur);
			tokenID = CALIBURN_T_IDENTIFIER;
		}
		else if (isOperator(txt[cur]))
		{
			tokenLen = findOp(txt, cur);
			tokenID = CALIBURN_T_OPERATOR;
		}
		else if (txt[cur] == '\"' || txt[cur] == '\'')
		{
			tokenLen = findStr(txt, cur, txt[cur]);
			tokenID = CALIBURN_T_LITERAL_STR;
		}
		else
		{
			//TODO complain about unknown character
		}

		std::string tokenStr = txt.substr(cur, tokenLen);
		if (tokenID == CALIBURN_T_IDENTIFIER)
		{
			tokenItr = tokenValues.find(tokenStr.c_str());

			if (tokenItr != tokenValues.end())
			{
				tokenID = (*tokenItr).second;
			}

		}
		
		tokens.push_back(Token(tokenStr, tokenID, line, col));
		cur += tokenLen;
		col += tokenLen;

	}

}