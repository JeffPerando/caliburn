
#pragma once

#include <exception>
#include <string>
#include <sstream>

#include "tokenizer.h"

namespace caliburn
{
	struct CompilerError : public std::exception
	{
		std::string const stage;
		std::string const message;
		uint64_t const line;
		uint64_t const column;

		CompilerError() : stage("GENERIC"), message("Unknown error"), line(0), column(0) {}
		CompilerError(std::string s, std::string m, uint64_t l, uint64_t c) :
			stage(s), message(m), line(l), column(c) {}
		
		const char* what() const
		{
			std::stringstream ss;

			ss << "[" << stage << "] ";
			ss << message;
			ss << " on line " << line << ":" << column;

			return ss.str().c_str();
		}

	};
	
	class TokenizerError : public CompilerError
	{
		TokenizerError(std::string m, Token* tkn) : CompilerError("TOKENIZER", m, tkn->line, tkn->column) {}

	};

	class ParserError : public CompilerError
	{
		ParserError(std::string m, Token* tkn) : CompilerError("PARSER", m, tkn->line, tkn->column) {}

	};

}