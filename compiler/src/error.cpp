
#include "error.h"

using namespace caliburn;

void Error::prettyPrint(ref<const TextDoc> doc, ref<std::stringstream> ss, bool color) const
{
	/*
	TODO:

	Consider using ParsedObj.prettyPrint()
	Maybe add more lines of code to give the user even more context
	Make lines configurable so users aren't bombarded with more context than needed
	*/

	constexpr auto ERR_LINE_COUNT = 3;

	ss << '[' << COMPILE_STAGES[(int)stage] << "] ";
	
	TextPos startPos, endPos;
	bool hasLine = true;

	if (problemTknStart != nullptr)
	{
		startPos = problemTknStart->pos;
		endPos = problemTknEnd->pos;
	}
	else
	{
		hasLine = false;
	}

	ss << "Error";

	if (hasLine)
	{
		ss << " on line " << startPos.line;
	}
	
	ss << ": " << message << '\n';

	if (contextStart != nullptr)
	{
		auto contextLine = contextStart->pos.line;

		ss << "Context:\n";
		ss << contextLine << ' ' << doc.getLine(contextStart->pos.line);

	}

	if (problemTknStart != nullptr && problemTknEnd != nullptr)
	{
		for (auto i = ERR_LINE_COUNT; i > 0; --i)
		{
			auto line = startPos.line - i;

			if (line > 0)
			{
				ss << line << '\t' << doc.getLine(line) << '\n';

			}

		}

		auto startLine = problemTknStart->pos.line;
		auto endLine = problemTknEnd->pos.line;

		auto endTknLen = problemTknEnd->textEnd - problemTknEnd->textStart;

		if (startLine == endLine)
		{
			if (color)
			{
				auto lineStr = doc.getLine(startPos.line);
				
				ss << startPos.line << '\t';

				if (startPos.column > 1)
				{
					ss << lineStr.substr(0, startPos.column - 1);
				}

				ss << "\033[1;31m";
				ss << doc.text.substr(problemTknStart->textStart, problemTknStart->textEnd - problemTknStart->textStart);
				ss << "\033[0m";

				if (lineStr.length() >= (startPos.column + endTknLen))
				{
					ss << lineStr.substr((startPos.column - 1) + endTknLen);
				}

			}
			else
			{
				ss << startPos.line << '\t' << doc.getLine(startPos.line) << '\n';
				ss << "\t";
				ss << std::string(startPos.column, ' ');
				ss << std::string(endTknLen, '^');

			}
			
		}
		else
		{
			auto startLineStr = doc.getLine(startPos.line);
			auto endLineStr = doc.getLine(endPos.line);

			if (startPos.column > 1)
			{
				ss << startLineStr.substr(0, startPos.column - 1);
			}

			ss << "\033[1;31m";
			ss << startPos.line << '\t' << startLineStr.substr(startPos.column - 1) << '\n';
			for (uint32_t line = startPos.line + 1; line < endPos.line; ++line)
			{
				ss << line << '\t' << doc.getLine(line) << '\n';
			}
			ss << endPos.line << '\t' << endLineStr.substr(0, endTknLen + endPos.column - 1);
			ss << "\033[0m";

			if (endLineStr.length() >= (endPos.column + endTknLen))
			{
				ss << endLineStr.substr((endPos.column - 1) + endTknLen);
			}

		}

		ss << '\n';

		for (auto i = 0; i < ERR_LINE_COUNT; ++i)
		{
			auto line = endPos.line + i + 1;

			if (line >= doc.getLineCount())
			{
				break;
			}

			ss << line << '\t' << doc.getLine(line) << '\n';

		}

	}
	
	if (!notes.empty())
	{
		ss << "\nNotes:";

		for (size_t i = 0; i < notes.size(); ++i)
		{
			ss << '\n\t' << (i + 1) << ". " << notes.at(i);

		}
		
	}

	ss << '\n';

}

sptr<Error> ErrorHandler::err(std::string msg, sptr<Token> tknStart, sptr<Token> tknEnd)
{
	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->problemTknStart = tknStart;
	e->problemTknEnd = tknEnd;

	errors.push_back(e);

	return e;
}
