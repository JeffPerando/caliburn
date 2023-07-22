
#pragma once

#include <string>
#include <vector>

#include "basic.h"

namespace caliburn
{
	struct TextPos
	{
		size_t line, column;

		TextPos() : line(1), column(0) {}
		TextPos(size_t l, size_t c = 0) : line(l), column(c) {}
		
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
			column += off;
		}

	};

	struct TextDoc
	{
		const std::string text;

		std::vector<size_t> lineOffsets{0};

		TextDoc(ref<const std::string> str) : text(str) {}

		void startLine(size_t off)
		{
			//TODO can't be bothered to write validation code
			lineOffsets.push_back(off);
		}

		std::string getLine(size_t line) const
		{
			if (line > lineOffsets.size())
			{
				return "";
			}

			//treat the line # as off by one
			size_t off = lineOffsets.at(line - 1);
			
			if (line == lineOffsets.size())
			{
				return text.substr(off);
			}

			size_t nextOff = lineOffsets.at(line);

			return text.substr(off, nextOff - off);
		}

	};

}