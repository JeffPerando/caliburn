
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
		void move(uint32_t off = 1)
		{
			column += off;
		}

		constexpr uint32_t getLine() const
		{
			return line + 1;
		}

	};

	/*
	A TextDoc is responsible for keeping track of lines within a string
	*/
	struct TextDoc
	{
		const std::string_view text;

		std::vector<std::string_view> lines;
		size_t lastOffset = 0;

		TextDoc(in<std::string> str) : text(str) {}

		/*
		Adds a new line at the given offset.

		This function assumes linear traversal of the text the document holds. As such,
		offsets must grow in size
		*/
		void startLine(size_t off)
		{
			if (off < lastOffset)
			{
				throw std::exception("You somehow tried to push an impossible line offset.");
			}

			lines.push_back(text.substr(lastOffset, off - 1));
			lastOffset = off;

		}

		/*
		Creates a substring of the given line, using the stored line offsets.

		The resulting line substring *shouldn't* contain a newline character.
		*/
		std::string_view getLine(in<TextPos> pos) const
		{
			return getLineDirect(pos.line);
		}

		std::string_view getLineDirect(size_t index) const
		{
			if (lines.empty())
			{
				return text;
			}

			if (index >= lines.size())
			{
				throw std::exception((std::stringstream() << "Invalid line: " << index).str().c_str());
			}

			return lines.at(index);
		}

		size_t getLineCount() const
		{
			return lines.size();
		}

	};

}