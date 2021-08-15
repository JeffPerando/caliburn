
#pragma once

#include <string>
#include <sstream>

#include "tokenizer.h"

namespace caliburn
{
	//Exists because C++ doesn't allow for enum conversion to string
	static const std::string COMPILE_STAGES[] =
		{"TOKENIZER", "PARSER", "COMPILER", "OPTIMIZER"};

	//Some of these shouldn't throw exceptions, but who knows?
	//Particularly the tokenizer; any character that isn't reserved is an identifier.
	enum class CompileStage
	{
		TOKENIZER,
		PARSER,
		COMPILER,
		OPTIMIZER
	};

	struct CaliburnException
	{
		CompileStage const stage;
		std::string const message;
		Token* const current;

		CaliburnException(CompileStage s, std::string m, Token* tkn) :
			stage(s), message(m), current(tkn) {}
		
		virtual void toStr(std::string& out)
		{
			std::stringstream ss;

			ss << '[';
			ss << COMPILE_STAGES[(int)stage];
			ss << "] ";
			ss << current->line;
			ss << ':';
			ss << current->column;
			ss << ' ';
			ss << message;

			out = ss.str();

		}

	};

	struct ParseException : public CaliburnException
	{
		ParseException(std::string m, Token* tkn) :
			CaliburnException(CompileStage::PARSER, m, tkn) {}

	};

	struct CompilerException : public CaliburnException
	{
		CompilerException(std::string m, Token* tkn) :
			CaliburnException(CompileStage::COMPILER, m, tkn) {}

	};

	struct InvalidDeclException : public ParseException
	{
		InvalidDeclException(Token* invalid) :
			ParseException((std::stringstream() << "invalid declaration: \"" << invalid->str << '\"').str(),
				current) {}

	};

	//USE THIS SPARINGLY
	//Please use a more descriptive exception when possible
	struct UnexpectedTokenException : public ParseException
	{
		UnexpectedTokenException(Token* current, char expected) :
			ParseException(
				(std::stringstream() << "expected \'" << expected << "\', found \'" << current->str << '\'').str(),
				current) {}

	};
	
}