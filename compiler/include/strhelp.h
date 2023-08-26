
#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "basic.h"

namespace caliburn
{
	struct TextPos
	{
		uint32_t line, column;

		TextPos() : line(1), column(0) {}
		TextPos(uint32_t l, uint32_t c = 0) : line(l), column(c) {}
		
		std::string toStr() const
		{
			std::stringstream ss;

			ss << '(' << line << ':' << column << ')';

			return ss.str();
		}

		bool operator<(ref<const TextPos> rhs) const
		{
			if (line > rhs.line)
			{
				return false;
			}

			return column < rhs.column;
		}

		void newline()
		{
			++line;
			column = 1;
		}

		//Would be named "next", but then it shares 2 letters with newline(),
		//and I can't be bothered to type "new" every time I want to call it
		void move(size_t off = 1)
		{
			column += (uint32_t)off;
		}

	};

	struct TextDoc
	{
		const std::string text;

		std::vector<size_t> lineOffsets{0};

		TextDoc(ref<const std::string> str) : text(str) {}

		void startLine(size_t off)
		{
			if (lineOffsets.size() > 1 && off <= lineOffsets.back())
			{
				throw std::exception("You somehow tried to push an impossible line offset.");
			}

			lineOffsets.push_back(off);
		}

		std::string getLine(size_t line) const
		{
			if (line >= lineOffsets.size())
			{
				return "";
			}

			//treat the line # as off by one
			size_t off = lineOffsets.at(line);
			
			if (line == lineOffsets.size())
			{
				return text.substr(off);
			}

			size_t nextOff = lineOffsets.at(line + 1);

			return text.substr(off, nextOff - off);
		}

	};

}