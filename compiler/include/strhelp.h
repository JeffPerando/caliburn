
#pragma once

#include <array>
#include <string>
#include <sstream>
#include <vector>

#include "basic.h"

namespace caliburn
{
	static const HashMap<char, uint8_t> HEX_DIGITS = {
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4},
		{'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9},
		{'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15},
		{'A', 10}, {'B', 11}, {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15}
	};

	static constexpr bool isDecInt(char ch)
	{
		return ch >= '0' && ch <= '9';
	}

	static inline uint64_t parseB10Int(std::string_view str)
	{
		uint64_t parsed = 0;

		for (size_t idx = 0; idx < str.length(); ++idx)
		{
			auto const digit = str[idx];

			if (digit == '_')
			{
				continue;
			}

			if (!isDecInt(digit))
			{
				break;
			}

			parsed = (parsed * 10) + static_cast<uint64_t>(digit - '0');

		}

		return parsed;
	}

	static inline uint64_t parseInt(std::string_view lit)
	{
		uint64_t parsedLit = 0;

		if (lit[0] == '0' && lit.length() > 2)
		{
			switch (std::toupper(lit[1]))
			{
			case 'X': {
				for (size_t idx = 2; idx < lit.length(); ++idx)
				{
					char digit = lit[idx];
					if (digit == '_')
					{
						continue;
					}

					if (auto num = HEX_DIGITS.find(digit); num != HEX_DIGITS.end())
					{
						parsedLit <<= 4;
						parsedLit |= static_cast<uint64_t>(num->second);
					}
					else
					{
						//TODO complain
					}

				}
			}; break;
			case 'B': {
				for (size_t idx = 2; idx < lit.length(); ++idx)
				{
					char digit = lit[idx];
					if (digit == '_')
					{
						continue;
					}

					parsedLit <<= 1;
					parsedLit |= static_cast<uint64_t>(digit - '0');

				}
			}; break;
			case 'C': {
				for (size_t idx = 2; idx < lit.length(); ++idx)
				{
					char digit = lit[idx];
					if (digit == '_')
					{
						continue;
					}

					parsedLit <<= 3;
					parsedLit |= static_cast<uint64_t>(digit - '0');

				}
			}; break;
			}

			return parsedLit;
		}

		return parseB10Int(lit);
	}

	static inline std::array<std::string_view, 2> splitStr(std::string_view str, std::string_view delim) noexcept
	{
		auto found = str.find_first_of(delim);

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + delim.length()) };
	}

	static inline std::array<std::string_view, 2> splitStr(std::string_view str, char delim) noexcept
	{
		auto found = str.find(delim);

		if (found == std::string::npos)
		{
			return { str, "" };
		}

		return { str.substr(0, found), str.substr(found + 1) };
	}

	static inline std::array<std::string_view, 2> splitStr(std::string_view str, std::vector<char> delims) noexcept
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
		const std::string_view text;

		std::vector<std::string_view> lines;
		
		TextDoc(in<std::string_view> str);

		std::string_view getLine(TextPos pos) const;

	};

}
