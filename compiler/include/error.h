
#pragma once

#include <string>
#include <sstream>

#include "syntax.h"

namespace caliburn
{
	static const std::vector<std::string> COMPILE_STAGES = {
		"Unknown", "Tokenizer", "Parsing", "AST Validation",
		"Cond. Compilation", "CLLR Generation", "CLLR Validation",
		"Target Compilation"
	};

	enum class CompileStage
	{
		UNKNOWN,
		TOKENIZER,
		PARSER,
		AST_VALIDATION,
		CONDITIONAL_COMPILATION,
		CLLR_EMIT,
		CLLR_VALIDATION,
		OUT_EMIT
	};

	struct Error
	{
		CompileStage stage = CompileStage::UNKNOWN;
		std::string message;
		sptr<Token> problemTknStart = nullptr;
		sptr<Token> problemTknEnd = nullptr;
		sptr<Token> contextStart = nullptr;
		std::vector<std::string> suggestions;

		void suggest(std::string idea)
		{
			suggestions.push_back(idea);
		}

		void prettyPrint(ref<const TextDoc> txt, ref<std::stringstream> ss, bool color) const;

		std::string toStr(ref<const TextDoc> txt, bool color)
		{
			std::stringstream ss;
			prettyPrint(txt, ss, color);
			return ss.str();
		}

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

		void printout(ref<std::vector<std::string>> out, ref<const TextDoc> doc, bool colored) const
		{
			for (auto const& e : errors)
			{
				out.push_back(e->toStr(doc, colored));
			}

		}

		sptr<Error> err(std::vector<std::string> msgs, ref<const ParsedObject> keyObj)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), keyObj);
		}

		sptr<Error> err(std::string msg, ref<const ParsedObject> keyObj)
		{
			return err(msg, keyObj.firstTkn(), keyObj.lastTkn());
		}

		sptr<Error> err(std::vector<std::string> msgs, sptr<Token> keyTkn)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), keyTkn);
		}

		sptr<Error> err(std::string msg, sptr<Token> keyTkn)
		{
			return err(msg, keyTkn, keyTkn);
		}

		sptr<Error> err(std::vector<std::string> msgs, sptr<Token> startTkn, sptr<Token> endTkn)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), startTkn, endTkn);
		}

		sptr<Error> err(std::string msg, sptr<Token> tknStart, sptr<Token> tknEnd);

	};
	
}
