
#pragma once

#include <string>
#include <vector>

#include "syntax.h"

namespace caliburn
{
	struct Token
	{
		std::string token;
		uint64_t identifier;
		uint64_t line, column;

		//the fact that this has to exist is bothersome
		Token(std::string t, uint64_t id, uint64_t l, uint64_t c) :
			token(t), identifier(id), line(l), column(c) {}

	};
	
	size_t findOp(std::string txt, uint64_t cur);

	size_t findStr(std::string txt, uint64_t cur, char delim);

	size_t findInt(std::string txt, uint64_t cur);

	size_t findIdentifier(std::string txt, uint64_t cur);

	bool isIntStart(char chr);

	bool isInt(char chr);

	bool isIdentifier(char chr);

	bool isOperator(char chr);

	void tokenize(std::string txt, std::vector<Token>& tokens);

}
