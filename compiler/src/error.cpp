
#include "error.h"

using namespace caliburn;

std::string Error::print(in<TextDoc> doc, sptr<const CompilerSettings> settings) const
{
	const uint32_t errLineCount = settings->errorContextLines;

	constexpr std::string_view ERR_TXT_START = "\033[41m";
	constexpr std::string_view ERR_TXT_END = "\033[0m";

	std::stringstream ss;

	ss << '[' << COMPILE_STAGES[SCAST<uint32_t>(stage)] << "] ";
	
	ss << "Error";

	if (startTkn.exists())
	{
		if (endTkn.pos.getLine() == startTkn.pos.getLine())
		{
			ss << " on line " << startTkn.pos.getLine();
		}
		else
		{
			ss << " on lines " << startTkn.pos.getLine() << '-' << endTkn.pos.getLine();
		}

	}
	
	ss << ": " << message << '\n';

	if (contextStart.exists())
	{
		auto const& cxtLine = doc.getLine(contextStart.pos);

		ss << "Context: \n";
		ss << contextStart.pos.getLine() << '\t' << cxtLine << '\n';
		ss << std::string(cxtLine.length(), '-');
		ss << '\n';
	}
	else
	{
		for (auto line = startTkn.pos.line - std::min(errLineCount, startTkn.pos.line); line < startTkn.pos.line; ++line)
		{
			ss << (line + 1) << '\t' << doc.lines[line] << '\n';
		}
	}
	
	if (startTkn.exists() && endTkn.exists())
	{
		if (startTkn.pos.line == endTkn.pos.line)
		{
			auto const lineStr = doc.getLine(startTkn.pos);

			ss << ERR_TXT_START;
			ss << startTkn.pos.getLine();
			ss << ERR_TXT_END;
			ss << '\t';
			ss << lineStr.substr(0, startTkn.pos.column);
			ss << ERR_TXT_START;
			ss << lineStr.substr(startTkn.pos.column, endTkn.pos.column + endTkn.str.length() - startTkn.pos.column);
			ss << ERR_TXT_END;
			ss << lineStr.substr(endTkn.pos.column + endTkn.str.length());

		}
		else
		{
			auto startLine = doc.getLine(startTkn.pos);
			auto endLine = doc.getLine(endTkn.pos);

			ss << ERR_TXT_START;
			ss << startTkn.pos.getLine();
			ss << ERR_TXT_END;
			ss << '\t';
			ss << startLine.substr(0, startTkn.pos.column);
			ss << ERR_TXT_START;
			ss << startTkn.str;
			ss << '\n';
			
			for (uint32_t line = startTkn.pos.line + 1; line < endTkn.pos.line; ++line)
			{
				ss << (line + 1) << '\t' << doc.lines[line] << '\n';
			}

			ss << endTkn.pos.getLine() << '\t' << endLine.substr(0, endTkn.pos.column + endTkn.str.length());
			ss << ERR_TXT_END;
			ss << endLine.substr(endTkn.pos.column + endTkn.str.length());
			
		}

		ss << '\n';

		for (uint32_t i = 1; i <= errLineCount; ++i)
		{
			auto line = endTkn.pos.line + i;

			if (line >= doc.lines.size())
			{
				break;
			}

			ss << (line + 1) << '\t' << doc.lines[line] << '\n';

		}
		
	}

	if (!notes.empty())
	{
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

	return ss.str();
}

sptr<Error> ErrorHandler::err(in<std::string> msg, in<Token> tknStart, in<Token> tknEnd)
{
	auto e = new_sptr<Error>();

	e->stage = stage;
	e->message = msg;
	e->startTkn = tknStart;
	e->endTkn = tknEnd;

	errors.push_back(e);

	return e;
}
