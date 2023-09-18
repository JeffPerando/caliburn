
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>

#include "tokenizer.h"

using namespace caliburn;

Tokenizer::Tokenizer(sptr<TextDoc> t) : doc(t), buf(std::vector<char>(t->text.begin(), t->text.end()))
{
	//I'm so sorry for this.

	asciiTypes.fill(CharType::UNKNOWN);

	for (auto i = 0; i <= 32; ++i)
	{
		asciiTypes[i] = CharType::WHITESPACE;
	}

	for (auto c = '0'; c <= '9'; ++c)
	{
		asciiTypes[c] = CharType::INT;
	}

	for (auto l = 'A'; l <= 'Z'; ++l)
	{
		asciiTypes[l] = CharType::IDENTIFIER;
		//'a' - 'z'
		auto cap = l + 32;
		asciiTypes[cap] = CharType::IDENTIFIER;
	}

	for (auto ch : OPERATOR_CHARS)
	{
		asciiTypes[ch] = CharType::OPERATOR;
	}

	asciiTypes['\''] = CharType::STRING_DELIM;
	asciiTypes['\"'] = CharType::STRING_DELIM;
	asciiTypes['#'] = CharType::COMMENT;
	asciiTypes['('] = CharType::SPECIAL;
	asciiTypes[')'] = CharType::SPECIAL;
	asciiTypes[','] = CharType::SPECIAL;
	asciiTypes['['] = CharType::SPECIAL;
	asciiTypes[']'] = CharType::SPECIAL;
	asciiTypes['{'] = CharType::SPECIAL;
	asciiTypes['}'] = CharType::SPECIAL;
	
	//character 127 (DEL) is a reserved UNKNOWN
	for (auto i = 0; i < 127; ++i)
	{
		if (asciiTypes[i] == CharType::UNKNOWN)
		{
			asciiTypes[i] = CharType::SPECIAL;
		}

	}

}

CharType Tokenizer::getType(char chr) const
{
	return asciiTypes[chr];
}

size_t Tokenizer::findIntLiteral(out<TokenType> type, out<std::string> lit)
{
	auto validIntChars = &DEC_INTS;

	auto const isValidInt = lambda(char chr)
	{
		return std::binary_search(validIntChars->begin(), validIntChars->end(), chr);
	};

	auto first = buf.cur();

	if (!isValidInt(first))
	{
		return 0;
	}

	size_t startIndex = buf.index();
	bool isPrefixed = true;
	bool isFloat = false;

	std::stringstream ss;

	//find either a hex, binary, or octal integer
	if (first == '0' && buf.hasRem(3))
	{
		char litPrefix = buf.peek(1);

		switch (litPrefix)
		{
			case 'x': pass;
			case 'X': validIntChars = &HEX_INTS; break;
			case 'b': pass;
			case 'B': validIntChars = &BIN_INTS; break;
			case 'c': pass;
			case 'C': validIntChars = &OCT_INTS; break;
			default: isPrefixed = false;
		}

		if (isPrefixed)
		{
			ss << first;
			ss << litPrefix;

			buf.consume(2);

		}

	}
	else isPrefixed = false;

	//get the actual numerical digits
	while (buf.hasCur())
	{
		auto digit = buf.cur();
		
		if (isValidInt(digit))
		{
			//Make all lowercase hex digits uppercase, for everyone's sanity's sake
			if (digit > 96)
				digit -= 32;
			ss << digit;
			buf.consume();
		}
		else if (digit == '_')
		{
			buf.consume();
		}
		else break;

	}

	//find a decimal/float
	if (!isPrefixed)
	{
		//Find the fractional component
		if (buf.hasRem(2) && buf.cur() == '.')
		{
			buf.consume();
			ss << '.';
			isFloat = true;

			while (buf.hasCur())
			{
				auto dec = buf.cur();

				if (isValidInt(dec))
				{
					ss << dec;
					buf.consume();
				}
				else break;

			}

		}

		/*
		Look for an exponent
		*/
		if (buf.hasRem(2))
		{
			auto exp = buf.cur();

			if (exp == 'e' || exp == 'E')
			{
				ss << exp;

				auto sign = buf.next();

				if (sign == '+' || sign == '-')
				{
					ss << sign;
					buf.consume();
				}

				while (buf.hasCur() && isValidInt(buf.cur()))
				{
					ss << buf.cur();
					buf.consume();
				}

			}

		}

	}

	auto typeSuffix = "int";
	auto width = 32;

	/*
	Caliburn int literals have C-like suffixes. This bit of code finds one.
	*/
	if (buf.hasCur())
	{
		auto suffix = buf.cur();
		buf.consume();

		//lowercase
		if (suffix > 96)
			suffix -= 32;

		if (suffix == 'F')
		{
			isFloat = true;
		}
		else if (suffix == 'D')
		{
			width = 64;
			isFloat = true;
		}
		else if (suffix == 'U')
		{
			typeSuffix = "uint";

			if (buf.cur() == 'l' || buf.cur() == 'L')
			{
				buf.consume();
				width = 64;
			}

		}
		else if (suffix == 'L')
		{
			width = 64;
		}
		else
		{
			buf.rewind();
		}
		
	}

	//This code is not useless, KEEP IT
	//remember earlier when we checked for a fractional part? Yeah...
	if (isFloat)
	{
		typeSuffix = "float";
	}

	//Attach type information to the end of the literal
	//No, I don't want to expose this syntactically. Those literals look so ugly.
	ss << '_' << typeSuffix << width;

	lit = ss.str();
	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;

	size_t len = (buf.currentIndex() - startIndex);

	//The literal might not be selected
	buf.revertTo(startIndex);

	return len;
}

size_t Tokenizer::findIdentifierLen()
{
	size_t len = 0;

	for (size_t i = 0; i < buf.remaining(); ++i)
	{
		auto type = getType(buf.peek(i));

		if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			++len;
		}
		else break;

	}

	return len;
}

std::vector<sptr<Token>> Tokenizer::tokenize()
{
	std::vector<sptr<Token>> tokens;

	while (buf.hasCur())
	{
		const size_t start = buf.index();
		const TextPos startPos = pos;
		const char current = buf.cur();

		//Caliburn currently only supports ASCII
		if (current > 127)
		{
			errors->err("Unidentifiable character", makeCharTkn(TokenType::UNKNOWN));
			buf.consume();
			pos.move();
			continue;
		}

		const CharType type = getType(current);

		auto tknType = TokenType::UNKNOWN;
		std::string tknContent = "";
		size_t tknLen = 0;

		if (type == CharType::WHITESPACE)
		{
			/*
			See https://en.wikipedia.org/w/index.php?title=Newline
			So there's 2 major line endings we care about:
			Windows \n
			Linux/MacOS \r\n
			Result: We don't care about \r. We just don't. We see it, we skip it. That way line counts are kept sane.
			*/

			if (current == '\n')
			{
				pos.newline();
				doc->startLine(buf.index() + 1);
			}
			else if (current != '\r')
			{
				pos.move();
			}

			buf.consume();
			continue;
		}
		else if (type == CharType::COMMENT)
		{
			//skip to the end of the line, which will later invoke the whitespace code above
			while (buf.hasCur())
			{
				char commentChar = buf.cur();

				if (commentChar == '\n')
				{
					break;
				}
				else if (commentChar != '\r')
				{
					pos.move();
				}

				buf.consume();

			}

			//go back to the start
			continue;
		}
		else if (type == CharType::IDENTIFIER || type == CharType::INT)
		{
			/*
			Identifiers in Caliburn can start with a number.

			First, we find the lengths of both an identifier and an integer. Whichever is
			longer is the one we go with. If they're the same, the integer is made since it
			should be a valid integer literal, which isn't a valid identifier.
			*/

			std::string intLit = "";
			
			const size_t wordLen = findIdentifierLen();
			const size_t intLen = findIntLiteral(tknType, intLit);

			if (intLen == 0 && wordLen == 0)
			{
				//I don't know how this could be possible. Maybe a code regression?
				errors->err("Zero-width identifier/integer found", makeCharTkn(TokenType::UNKNOWN));
				buf.consume();
				pos.move();
				continue;
			}

			if (wordLen > intLen)
			{
				tknContent = doc->text.substr(buf.index(), wordLen);
				tknLen = wordLen;
				tknType = TokenType::IDENTIFIER;

				if (KEYWORDS.count(tknContent))
				{
					tknType = TokenType::KEYWORD;
				}

			}
			else
			{
				tknContent = intLit;
				tknLen = intLen;

			}

		}
		else if (type == CharType::STRING_DELIM)
		{
			//only used for error messaging
			sptr<Token> delimTkn = makeCharTkn(TokenType::UNKNOWN);

			std::stringstream ss;
			char delim = current;
			bool foundDelim = false;

			buf.consume();

			while (buf.hasCur())
			{
				char strChar = buf.cur();

				if (strChar == delim)
				{
					buf.consume();
					pos.move();
					foundDelim = true;
					break;
				}

				if (strChar == '\\')
				{
					//Escaped character; skip the backslash and whatever comes after
					//Note: This will be sound when UTF-8 support is added, since UTF literals won't be needed

					buf.consume(2);
					pos.move(2);
					continue;
				}
				
				if (strChar == '\n')
				{
					//Newlines are *not* skipped, but trailing whitespace after is
					ss << '\n';
					doc->startLine(buf.index() + 1);
					buf.consume();
					pos.newline();
					
					while (buf.hasCur())
					{
						char ws = buf.cur();

						if (getType(ws) != CharType::WHITESPACE)
							break;

						if (ws == '\n')
						{
							break;
						}
						else if (ws != '\r')
						{
							buf.consume();
							pos.move();
						}

					}

					continue;
				}

				ss << strChar;
				buf.consume();
				pos.move();

			}

			if (!foundDelim)
			{
				errors->err("Unescaped string", delimTkn);
				break;
			}

			tknType = TokenType::LITERAL_STR;
			tknContent = ss.str();
			//DO NOT SET TOKEN LENGTH

		}
		else if (type == CharType::SPECIAL)
		{
			/*
			Special tokens are always 1-char tokens and typically have an override (see TOKEN_TYPE_OVERRIDES)
			*/
			tknContent = std::string(1, current);
			tknLen = 1;
		}
		else if (type == CharType::OPERATOR)
		{
			/*
			* The tokenizer here looks ahead to find the longest possible operator, then will pare back the substring
			* until a valid operator is found.
			*/

			size_t opLen = 1;

			while (buf.hasRem(opLen))
			{
				if (getType(buf.peek(opLen)) != CharType::OPERATOR)
				{
					break;
				}

				++opLen;

			}
			
			while (opLen > 1)
			{
				const auto testOp = doc->text.substr(buf.index(), opLen);
				
				if (LONG_OPS.count(testOp))
				{
					tknType = TokenType::OPERATOR;
					tknContent = testOp;
					tknLen = opLen;
					break;
				}

				--opLen;

			}

			//Couldn't find anything longer, just pass it along as a char token
			if (opLen == 1)
			{
				tknType = TokenType::OPERATOR;
				tknContent = std::string(1, current);
				tknLen = 1;
			}

		}
		else
		{
			//if all else fails, skip it.
			buf.consume();
			pos.move();
			continue;
		}

		if (auto typeOverride = TOKEN_TYPE_OVERRIDES.find(tknContent); typeOverride != TOKEN_TYPE_OVERRIDES.end())
		{
			tknType = typeOverride->second;
		}

		buf.consume(tknLen);
		pos.move(tknLen);

		tokens.push_back(new_sptr<Token>(tknContent, tknType, startPos, start, buf.index()));

	}

	return tokens;
}
