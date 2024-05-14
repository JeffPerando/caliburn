
#pragma once

#include <string>
#include <sstream>

#define CBRN_NO_IMPORT
#include "caliburn.h"
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
		SYMBOL_GENERATION,
		CLLR_EMIT,
		CLLR_VALIDATION,
		OUT_EMIT
	};

	/*
	Strings for the CompileStage enum
	*/
	static const std::vector<std::string> COMPILE_STAGES = {
		"Unknown", "Tokenizing", "Parsing", "AST Validation",
		"Cond. Compilation", "Symbol Generation", "CLLR Generation", "CLLR Validation",
		"Target Compilation"
	};

	struct Error
	{
		CompileStage stage = CompileStage::UNKNOWN;
		std::string message;
		Token problemTknStart;
		Token problemTknEnd;
		/*
		This token is only used to give the user a line of code with more context in it.
		Such context may be a statement containing the problematic code
		*/
		Token contextStart;
		//Notes are used to give the user potentially helpful information, such as context or common solutions
		std::vector<std::string> notes;

		void note(in<std::string> idea)
		{
			notes.push_back(idea);
		}

		void note(in<std::vector<std::string>> idea_list)
		{
			std::stringstream ss;

			for (auto const& item : idea_list)
				ss << item << ' ';

			notes.push_back(ss.str());

		}

		void prettyPrint(in<TextDoc> txt, out<std::stringstream> ss, bool color) const;

		std::string toStr(in<TextDoc> txt, bool color)
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
	private:
		sptr<const CompilerSettings> settings;
	public:
		const CompileStage stage;

		std::vector<sptr<Error>> errors;

		ErrorHandler(CompileStage s) : stage(s), settings(new_sptr<CompilerSettings>()) {}

		ErrorHandler(CompileStage s, sptr<const CompilerSettings> cs) : stage(s), settings(cs) {}

		bool empty() const
		{
			return errors.empty();
		}

		void dump(out<std::vector<sptr<Error>>> out) const
		{
			out.insert(out.end(), errors.begin(), errors.end());
		}

		void printout(out<std::vector<std::string>> out, in<TextDoc> doc) const
		{
			for (auto const& e : errors)
			{
				out.push_back(e->toStr(doc, settings->coloredErrors));
			}

		}

		//Error-generation methods beyond this point

		sptr<Error> err(in<std::vector<std::string_view>> msgs, in<ParsedObject> keyObj)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), keyObj);
		}

		sptr<Error> err(in<std::string> msg, in<ParsedObject> keyObj)
		{
			return err(msg, keyObj.firstTkn(), keyObj.lastTkn());
		}

		sptr<Error> err(in<std::vector<std::string_view>> msgs, in<Token> keyTkn)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), keyTkn);
		}

		sptr<Error> err(in<std::string> msg, in<Token> keyTkn)
		{
			return err(msg, keyTkn, keyTkn);
		}

		sptr<Error> err(in<std::vector<std::string_view>> msgs, in<Token> startTkn, in<Token> endTkn)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), startTkn, endTkn);
		}

		sptr<Error> err(in<std::vector<std::string>> msgs)
		{
			return err(msgs, Token());
		}

		sptr<Error> err(in<std::vector<std::string>> msgs, in<Token> tkn)
		{
			std::stringstream ss;

			for (auto const& msg : msgs)
				ss << msg << ' ';

			return err(ss.str(), tkn, tkn);
		}

		sptr<Error> err(in<std::string> msg, in<Token> tknStart, in<Token> tknEnd);

	};
	
}
