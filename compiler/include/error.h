
#pragma once

#include <string>
#include <sstream>

#include "syntax.h"

namespace caliburn
{
	/*
	Caliburn's compilation process is fairly straightforward. Errors during compilation
	will present the user with the stage the error was generated at.
	*/
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

	/*
	Strings for the CompileStage enum
	*/
	static const std::vector<std::string> COMPILE_STAGES = {
		"Unknown", "Tokenizing", "Parsing", "AST Validation",
		"Cond. Compilation", "CLLR Generation", "CLLR Validation",
		"Target Compilation"
	};

	struct Error
	{
		CompileStage stage = CompileStage::UNKNOWN;
		std::string message;
		sptr<Token> problemTknStart = nullptr;
		sptr<Token> problemTknEnd = nullptr;
		/*
		This token is only used to give the user a line of code with more context in it.
		Such context may be a statement containing the problematic code
		*/
		sptr<Token> contextStart = nullptr;
		//Notes are used to give the user potentially helpful information, such as context or common solutions
		std::vector<std::string> notes;

		void note(std::string idea)
		{
			notes.push_back(idea);
		}

		void note(std::vector<std::string> idea_list)
		{
			std::stringstream ss;

			for (auto const& item : idea_list)
				ss << item << ' ';

			notes.push_back(ss.str());

		}

		void prettyPrint(ref<const TextDoc> txt, ref<std::stringstream> ss, bool color) const;

		std::string toStr(ref<const TextDoc> txt, bool color)
		{
			std::stringstream ss;
			prettyPrint(txt, ss, color);
			return ss.str();
		}

	};

	/*
	Caliburn does not have a single error handler; rather, every compiler stage manages its own errors.

	Also note that errors are handled via shared pointers. This is to enable for the creation of notes after
	initial error creation.
	*/
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

		//Error-generation methods beyond this point

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
