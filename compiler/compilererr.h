
#pragma once

#include <string>
#include <sstream>

#include "syntax.h"

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
		sptr<Token> const current;
		std::vector<std::string> extras;

		CaliburnException(CompileStage s, std::string m, sptr<Token> tkn) :
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

			for (auto extra : extras)
			{
				ss << '\t' << extra << '\n';
			}

			out = ss.str();

		}

	};

	struct ParseException : public CaliburnException
	{
		ParseException(std::string m, sptr<Token> tkn) :
			CaliburnException(CompileStage::PARSER, m, tkn) {}

	};

	struct CompilerException : public CaliburnException
	{
		CompilerException(std::string m, sptr<Token> tkn) :
			CaliburnException(CompileStage::COMPILER, m, tkn) {}

	};

	struct InvalidDeclException : public ParseException
	{
		InvalidDeclException(sptr<Token> invalid) :
			ParseException((std::stringstream() << "invalid declaration: \"" << invalid->str << '\"').str(),
				current) {}

	};

	//USE THIS SPARINGLY
	//Please use a more descriptive exception when possible
	struct UnexpectedTokenException : public ParseException
	{
		UnexpectedTokenException(sptr<Token> current, char expected) :
			ParseException(
				(std::stringstream() << "expected \'" << expected << "\', found \'" << current->str << '\'').str(),
				current) {}

	};
	
}