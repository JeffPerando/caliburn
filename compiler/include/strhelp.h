
#pragma once

#include <array>
#include <string>
#include <sstream>
#include <vector>

#include "basic.h"

namespace caliburn
{
	inline std::array<std::string, 2> splitStr(in<std::string> str, in<std::string> delim) noexcept
	{
		auto found = str.find_first_of(delim);

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + delim.length()) };
	}

	/*
	Stores a line and column within a text file.

	Both start at 0, but visually add 1 (see toStr()) to be more user-presentable
	*/
	struct TextPos
	{
		uint32_t line = 0;
		uint32_t column = 0;

		TextPos() = default;
		TextPos(uint32_t l, uint32_t c = 0) : line(l), column(c) {}
		
		std::string toStr() const
		{
			std::stringstream ss;

			ss << '(' << (line + 1) << ':' << (column + 1) << ')';

			return ss.str();
		}

		bool operator<(in<TextPos> rhs) const
		{
			if (line > rhs.line)
			{
				return false;
			}

			return column < rhs.column;
		}

		/*
		Sets the position to the start of a new line
		*/
		void newline()
		{
			++line;
			column = 0;
		}

		/*
		Moves the position by a given number of characters
		
		Would be named "next", but then it shares 2 letters with newline(),
		and I can't be bothered to type "new" every time I want to call it
		*/
		void move(size_t off = 1)
		{
			column += (uint32_t)off;
		}

	};

	/*
	A TextDoc is responsible for keeping track of lines within a string
	*/
	struct TextDoc
	{
		const std::string text;

		//note: all text files have at least one line; adding a 0 to the start enables getLine() to always work.
		std::vector<size_t> lineOffsets{0};

		TextDoc(in<std::string> str) : text(str) {}

		/*
		Adds a new line at the given offset.

		This function assumes linear traversal of the text the document holds. As such,
		offsets must grow in size
		*/
		void startLine(size_t off)
		{
			if (lineOffsets.size() > 1 && off <= lineOffsets.back())
			{
				throw std::exception("You somehow tried to push an impossible line offset.");
			}

			lineOffsets.push_back(off);
		}

		/*
		Creates a substring of the given line, using the stored line offsets.

		The resulting line substring *shouldn't* contain a newline character.
		*/
		std::string getLine(size_t line) const
		{
			if (line >= lineOffsets.size())
			{
				throw std::exception((std::stringstream() << "Invalid line: " << line).str().c_str());
			}

			size_t off = lineOffsets.at(line);
			
			if ((line + 1) == lineOffsets.size())
			{
				return text.substr(off);
			}

			size_t nextOff = lineOffsets.at(line + 1);

			return text.substr(off, nextOff - off - 1);
		}

		size_t getLineCount() const
		{
			return lineOffsets.size();
		}

		size_t getLineSize(size_t line)
		{
			if (line >= lineOffsets.size())
			{
				return 0;
			}

			if ((line + 1) >= lineOffsets.size())
			{
				return text.length() - lineOffsets[line];
			}

			return lineOffsets[line + 1] - lineOffsets[line];
		}

	};

}