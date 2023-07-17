
#pragma once

#include <string>
#include <sstream>

#include "syntax.h"

namespace caliburn
{
	static const std::vector<std::string> COMPILE_STAGES =
		{"Tokenizer", "Parsing", "AST Validation", "Cond. Compilation", "CLLR Generation", "CLLR Validation", "Target Compilation"};

	enum class CompileStage
	{
		UNKNOWN,
		TOKENIZER,
		PARSER,
		AST_VALIDATION,
		CONDITIONAL_COMPILATION,
		CLLR_EMIT,
		CLLR_VALID,
		CLLR_OPTIMIZE,
		OUT_EMIT
	};

	struct Error
	{
		CompileStage stage = CompileStage::UNKNOWN;
		std::string message;
		TextPos problemPos;
		sptr<ParsedObject> problemObj = nullptr;
		sptr<Token> problemTkn = nullptr;
		std::vector<std::string> suggestions;

		void suggest(std::string idea)
		{
			suggestions.push_back(idea);
		}

		void prettyPrint(ref<const TextDoc> txt, ref<std::stringstream> ss);

	};

	struct ErrorHandler
	{
		const CompileStage stage;
		std::vector<sptr<Error>> errors;

		ErrorHandler(CompileStage s) : stage(s) {}

		bool empty() const
		{
			return errors.empty();
		}

		void dump(ref<std::vector<sptr<Error>>> out) const
		{
			out.insert(out.end(), errors.begin(), errors.end());
		}

		sptr<Error> err(std::string msg, ref<const TextPos> pos);
		sptr<Error> err(std::string msg, sptr<Token> keyTkn, sptr<ParsedObject> keyObj = nullptr);

	};
	
}