
#pragma once

#include <string>
#include <vector>

#include "syntax.h"

namespace caliburn
{
	using FindFunc = bool(*)(char chr);

	inline bool isIdentifier(char chr);

	inline bool isComment(char chr);

	inline bool isWhitespace(char chr);

	inline bool isOperator(char chr);

	inline bool isStrDelim(char chr);

	inline bool isDecInt(char chr);

	inline bool isHexInt(char chr);

	inline bool isOctInt(char chr);

	inline bool isBinInt(char chr);

	inline bool isSpecial(char chr);

	inline TokenType getSpecial(char chr);

	size_t find(FindFunc func, std::string& txt, uint64_t cur);

	char* findStr(std::string& txt, uint64_t& cur, uint64_t& line);

	size_t findIntLiteral(std::string& txt, uint64_t cur, TokenType& type);

	void tokenize(std::string& txt, std::vector<Token>& tokens);

}
