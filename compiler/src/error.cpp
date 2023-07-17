
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

	ss << '[' << COMPILE_STAGES[(int)stage] << "] ";
	
	if (problemTkn == nullptr)
	{
		ss << "Error on line " << problemPos.line << ": " << message;

		if (problemObj != nullptr)
		{
			ss << '\n';
			auto count = problemObj->totalParsedSize();

			if (count > 300)//idk, arbitrary limit
			{
				ss << "Object too big, see the code starting with:\n";
				ss << doc.getLine(problemObj->firstTkn()->pos.line);

			}
			else
			{
				ss << "See the following:\n";
				ss << doc.text.substr(problemObj->firstTkn()->textStart, count);

			}

		}

	}
	else
	{
		auto const& pos = problemTkn->pos;

		ss << "Error on line " << pos.line << ": ";
		ss << message << '\n';

		if (problemObj != nullptr)
		{
			auto const& objPos = problemObj->firstTkn()->pos;

			ss << doc.getLine(objPos.line) << '\n';

			if (pos.line - objPos.line > 1)
			{
				ss << "...\n";
			}

		}

		ss << doc.getLine(pos.line) << '\n';

		for (size_t i = 0; i < pos.column; ++i)
		{
			ss << ' ';
		}

		//off is only for counting the current index to the actual token
		for (size_t off = problemTkn->textStart; off < problemTkn->textEnd; ++off)
		{
			ss << '^';
		}

		if (problemObj != nullptr)
		{
			auto const& objPos = problemObj->lastTkn()->pos;

			if (objPos.line - pos.line > 1)
			{
				ss << "...\n";
			}

			ss << doc.getLine(objPos.line) << '\n';

		}

	}

	if (!suggestions.empty())
	{
		ss << "\nSuggestions: ";
		for (size_t i = 0; i < suggestions.size(); ++i)
		{
			ss << '\n\t' << (i + 1) << ". " << suggestions.at(i);

		}

	}

	ss << '\n';

}

sptr<Error> ErrorHandler::err(std::string msg, ref<const TextPos> pos)
{
	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->problemPos = pos;

	errors.push_back(e);

	return e;
}

sptr<Error> ErrorHandler::err(std::string msg, sptr<Token> keyTkn, sptr<ParsedObject> keyObj)
{
	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->problemPos = keyTkn->pos;
	e->problemTkn = keyTkn;
	e->problemObj = keyObj;

	errors.push_back(e);

	return e;
}
