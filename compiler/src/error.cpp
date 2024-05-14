
#include "error.h"

using namespace caliburn;

void Error::prettyPrint(in<TextDoc> doc, out<std::stringstream> ss, bool color) const
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
	
	ss << "Error";

	if (problemTknStart.exists())
	{
		startPos = problemTknStart.pos;
		endPos = problemTknEnd.pos;
		ss << " on line " << startPos.getLine();
	}
	
	ss << ": " << message;

	if (contextStart.exists())
	{
		ss << "\nContext:\n";
		ss << contextStart.pos.getLine() << ' ' << doc.getLine(contextStart.pos);

	}
	
	if (problemTknStart.exists() && problemTknEnd.exists())
	{
		ss << '\n';

		for (auto i = ERR_LINE_COUNT; i > 0; --i)
		{
			auto line = startPos.line - i;

			if (line > 0 && line < startPos.line)
			{
				ss << line << '\t' << doc.getLineDirect(line) << '\n';

			}

		}

		if (startPos.line == endPos.line)
		{
			if (color)
			{
				auto lineStr = doc.getLine(startPos);
				
				ss << (startPos.line + 1) << '\t';

				if (startPos.column > 0)
				{
					ss << lineStr.substr(0, startPos.column);
				}

				ss << "\033[1;31m";
				ss << problemTknStart.str;
				ss << "\033[0m";

				if (lineStr.length() > (startPos.column + problemTknEnd.str.length()))
				{
					ss << lineStr.substr(startPos.column + problemTknEnd.str.length());
				}

			}
			else
			{
				ss << startPos.line << '\t' << doc.getLine(startPos);
				ss << "\t";
				ss << std::string(startPos.column, ' ');
				ss << std::string(problemTknEnd.str.length(), '^');
			}
			
		}
		else
		{
			auto startPosStr = doc.getLine(startPos);
			auto endPosStr = doc.getLine(endPos);

			if (startPos.column > 0)
			{
				ss << startPosStr.substr(0, startPos.column);
			}

			ss << "\033[1;31m";
			ss << startPos.line << '\t' << endPosStr.substr(startPos.column - 1) << '\n';
			for (uint32_t line = startPos.line + 1; line < endPos.line; ++line)
			{
				ss << line << '\t' << doc.getLineDirect(line);
			}
			ss << endPos.line << '\t' << endPosStr.substr(0, problemTknEnd.str.length() + endPos.column - 1);
			ss << "\033[0m";

			if (endPosStr.length() >= (endPos.column + problemTknEnd.str.length()))
			{
				ss << endPosStr.substr((endPos.column - 1) + problemTknEnd.str.length());
			}

		}

		for (auto i = 0; i < ERR_LINE_COUNT; ++i)
		{
			auto line = endPos.line + i;

			if (line >= doc.getLineCount())
			{
				break;
			}

			ss << line << '\t' << doc.getLineDirect(line);

		}

	}

	if (!notes.empty())
	{
		ss << '\n';

		if (notes.size() == 1)
		{
			ss << "Note: " << notes.front();
		}
		else
		{
			ss << "Notes:";

			for (size_t i = 0; i < notes.size(); ++i)
			{
				ss << "\n\t" << (i + 1) << ". " << notes.at(i);

			}

		}

	}

}

sptr<Error> ErrorHandler::err(in<std::string> msg, in<Token> tknStart, in<Token> tknEnd)
{
	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->problemTknStart = tknStart;
	e->problemTknEnd = tknEnd;

	errors.push_back(e);

	return e;
}
