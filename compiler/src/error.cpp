
#include "error.h"

using namespace caliburn;

void Error::prettyPrint(ref<const TextDoc> doc, ref<std::stringstream> ss) const
{
	/*
	TODO:

	Consider using ParsedObj.prettyPrint()
	Maybe add more lines of code to give the user even more context
	Make lines configurable so users aren't bombarded with more context than needed
	*/

	constexpr auto ERR_LINE_COUNT = 3;

	ss << '[' << COMPILE_STAGES[(int)stage] << "] ";
	
	TextPos pos;
	bool hasLine = true;

	if (problemTknStart != nullptr)
	{
		pos = problemTknStart->pos;
	}
	else
	{
		hasLine = false;
	}

	ss << "Error";

	if (hasLine)
	{
		ss << " on line " << pos.line;
	}
	
	ss << ": " << message << '\n';

	if (contextStart != nullptr)
	{
		auto contextLine = contextStart->pos.line;

		ss << "Context:\n";
		ss << contextLine << ' ' << doc.getLine(contextStart->pos.line) << '\n';

	}

	if (problemTknStart != nullptr && problemTknEnd != nullptr)
	{
		for (auto i = ERR_LINE_COUNT; i > 0; --i)
		{
			auto line = pos.line - i;

			if (line > 0)
			{
				ss << line << ' ' << doc.getLine(line) << '\n';

			}

		}

		auto startLine = problemTknStart->pos.line;
		auto endLine = problemTknEnd->pos.line;

		if (startLine == endLine)
		{
			ss << pos.line << ' ' << doc.getLine(pos.line) << '\n';

			for (size_t i = 0; i < pos.column; ++i)
				ss << ' ';

			//off is only for counting the current index to the actual token
			for (size_t off = problemTknStart->textStart; off < problemTknEnd->textEnd; ++off)
				ss << '^';

			ss << '\n';

		}
		else //TODO this entire procedure looks uh... *goofy*
		{
			auto endTknLen = problemTknEnd->textEnd - problemTknEnd->textStart;

			auto txtStart = problemTknStart->pos.column;
			auto txtEnd = problemTknEnd->pos.column + endTknLen;

			auto start = problemTknStart->textStart;
			auto end = problemTknEnd->textEnd;

			ss << doc.text.substr(start, end - start) << '\n';

			for (size_t i = 0; i < txtStart; ++i)
				ss << ' ';

			for (size_t off = txtStart; off < txtEnd; ++off)
				ss << '^';

		}

		for (auto i = 0; i < ERR_LINE_COUNT; ++i)
		{
			auto line = pos.line + i;

			if (line == doc.getLineCount())
			{
				break;
			}

			ss << line << ' ' << doc.getLine(line) << '\n';

		}

	}
	
	if (!suggestions.empty())
	{
		ss << "\nSuggestions:";

		for (size_t i = 0; i < suggestions.size(); ++i)
		{
			ss << '\n\t' << (i + 1) << ". " << suggestions.at(i);

		}
		
	}

	ss << '\n';

}

sptr<Error> ErrorHandler::err(std::string msg, sptr<Token> tknStart, sptr<Token> tknEnd)
{
	if (tknStart == nullptr)
	{
		throw std::exception("Problem start token is null");
	}

	if (tknEnd == nullptr)
	{
		throw std::exception("Problem end token is null");
	}

	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->problemTknStart = tknStart;
	e->problemTknEnd = tknEnd;

	errors.push_back(e);

	return e;
}
