
#pragma once

#include <array>
#include <string>
#include <sstream>
#include <vector>

#include "basic.h"

namespace caliburn
{
	inline char chrToLower(char c) noexcept
	{
		return c + 32;
	}

	inline char chrToUpper(char c) noexcept
	{
		return c - 32;
	}

	inline uint64_t parseInt(std::string_view str)
	{
		return parseInt(str, str.length());
	}

	inline uint64_t parseInt(std::string_view str, size_t len)
	{
		len = (str.length() < len) ? str.length() : len;

		uint64_t parsed = 0;

		for (size_t idx = 0; idx < len; ++idx)
		{
			char digit = str[idx];

			if (digit == '_')
			{
				continue;
			}

			parsed *= 10;
			parsed += digit - '0';

		}

		return parsed;
	}

	inline std::array<std::string_view, 2> splitStr(std::string_view str, std::string_view delim) noexcept
	{
		auto found = str.find_first_of(delim);

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + delim.length()) };
	}

	inline std::array<std::string_view, 2> splitStr(std::string_view str, char delim) noexcept
	{
		auto found = str.find(delim);

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + 1) };
	}

	inline std::array<std::string_view, 2> splitStr(std::string_view str, std::vector<char> delims) noexcept
	{
		size_t found = std::string::npos;

		for (auto& ch : delims)
		{
			auto idx = str.find(ch);

			if (idx < found)
			{
				found = idx;
			}

		}

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + 1) };
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
		const std::string text;

		std::vector<std::string_view> lines;
		
		TextDoc(in<std::string> str);

		/*
		Retrieves a line using the line found in the passed text position.

		The resulting line substring *shouldn't* contain a newline character.
		*/
		std::string_view getLine(TextPos pos) const;

	};

}