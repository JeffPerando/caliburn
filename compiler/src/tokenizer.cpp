
#include <algorithm>
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

std::string Tokenizer::findStr()
{
	char delim = buf.cur();
	sptr<Token> startTkn = makeCharTkn(TokenType::UNKNOWN);

	std::stringstream ss;
	bool foundDelim = false;

	while (buf.hasCur())
	{
		char current = buf.cur();

		if (current == '\\')
		{
			//Escaped character; skip the backslash and whatever comes after
			buf.consume(2);
			pos.move(2);
			continue;
		}

		/*
		Newlines are skipped in string literals, and trailing whitespace is also skipped.

		Note to self: consider just writing a function for this since it shares some logic with whitespace
		*/
		if (current == '\n')
		{
			pos.newline();
			doc->startLine(buf.currentIndex() + 1);

			while (buf.hasCur())
			{
				char ws = buf.next();

				if (getType(ws) != CharType::WHITESPACE)
					break;

				if (ws == '\n')
				{
					pos.newline();
					doc->startLine(buf.currentIndex() + 1);

				}
				else
				{
					pos.move();
				}

			}

			continue;
		}

		if (current == delim)
		{
			buf.consume();
			pos.move();
			foundDelim = true;
			break;
		}

		ss << current;
		buf.consume();
		pos.move();

	}

	if (!foundDelim)
	{
		errors->err("Unescaped string", startTkn);
		return "";
	}

	return ss.str();
}

size_t Tokenizer::findIntLiteral(ref<TokenType> type, ref<std::string> lit)
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

	size_t startIndex = buf.currentIndex();
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
	for (size_t i = 0; i < buf.remaining(); ++i)
	{
		auto type = getType(buf.peek(i));

		if (type != CharType::IDENTIFIER && type != CharType::INT)
		{
			return i;
		}

	}

	return 0;
}

std::vector<sptr<Token>> Tokenizer::tokenize()
{
	std::vector<sptr<Token>> tokens;

	while (buf.hasCur())
	{
		auto current = buf.cur();

		if (current > 127)
		{
			errors->err("Unidentifiable character", makeCharTkn(TokenType::UNKNOWN));
			buf.consume();
			pos.move();
			continue;
		}

		CharType type = getType(current);

		//See https://en.wikipedia.org/w/index.php?title=Newline

		//So there's 2 major line endings we care about:
		//Windows \n
		//Linux/MacOS \r\n
		//Result: We don't care about \r. We just don't. We see it, we skip it. That way line counts are kept sane.
		if (type == CharType::WHITESPACE)
		{
			if (current == '\r')
			{
				//don't increment pos, just pretend we didn't see it
				//NOTE: with the increased usage of TextPos for strings, this might be a dumb idea.
				buf.consume();
				continue;
			}

			if (significantWhitespace)
			{
				tokens.push_back(makeCharTkn(TokenType::WHITESPACE));
			}

			if (current == '\n')
			{
				pos.newline();
				doc->startLine(buf.currentIndex() + 1);
			}
			else
			{
				pos.move();
			}

			buf.consume();

		}
		else if (type == CharType::COMMENT)
		{
			//skip to the end of the line, which will later invoke the whitespace code above
			while (buf.hasCur())
			{
				if (buf.cur() == '\n')
				{
					break;
				}

				pos.move();
				buf.consume();

			}

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
			TokenType intType = TokenType::UNKNOWN;

			size_t wordOffset = findIdentifierLen();
			size_t intOffset = findIntLiteral(intType, intLit);

			if (intOffset == 0 && wordOffset == 0)
			{
				//I don't know how this could be possible. Maybe a code regression?
				errors->err("Zero-width identifier/integer found", makeCharTkn(TokenType::UNKNOWN));
				buf.consume();
				pos.move();
				continue;
			}

			if (wordOffset > intOffset)
			{
				auto wordStr = doc->text.substr(buf.currentIndex(), wordOffset);
				auto wordType = TokenType::IDENTIFIER;

				//Looks for a keyword
				if (std::binary_search(KEYWORDS.begin(), KEYWORDS.end(), wordStr))
				{
					auto foundType = STR_TOKEN_TYPES.find(wordStr);

					if (foundType == STR_TOKEN_TYPES.end())
					{
						wordType = TokenType::KEYWORD;
					}
					else
					{
						wordType = foundType->second;
					}

				}

				tokens.push_back(new_sptr<Token>(wordStr, wordType, pos, buf.currentIndex(), wordOffset));
				buf.consume(wordOffset);
				pos.move(wordOffset);

			}
			else
			{
				tokens.push_back(new_sptr<Token>(intLit, intType, pos, buf.currentIndex(), intOffset));
				buf.consume(intOffset);
				pos.move(intOffset);

			}

		}
		else if (type == CharType::STRING_DELIM)
		{
			auto start = buf.currentIndex();
			auto str = findStr();

			//findStr should offset col and line
			tokens.push_back(new_sptr<Token>(str, TokenType::LITERAL_STR, pos, start, buf.currentIndex() - start));
			
		}
		else if (type == CharType::SPECIAL)
		{
			auto specialType = CHAR_TOKEN_TYPES.find(current);

			if (specialType != CHAR_TOKEN_TYPES.end())
			{
				tokens.push_back(makeCharTkn(specialType->second));
			}
			else
			{
				//TODO warn
			}
			
			buf.consume();
			pos.move();

		}
		else if (type == CharType::OPERATOR)
		{
			size_t opLen = 1;

			while (buf.hasRem(opLen))
			{
				if (getType(buf.peek(opLen)) != CharType::OPERATOR)
				{
					break;
				}

				++opLen;

			}
			
			while (opLen > 0)
			{
				auto testOp = doc->text.substr(buf.currentIndex(), opLen);
				auto meaning = INFIX_OPS.find(testOp);

				if (meaning != INFIX_OPS.end())
				{
					tokens.push_back(new_sptr<Token>(testOp, TokenType::OPERATOR, pos, buf.currentIndex(), opLen));
					buf.consume(opLen);
					pos.move(opLen);
					break;
				}

				auto specMeaning = SPECIAL_OPS.find(testOp);

				if (specMeaning != SPECIAL_OPS.end())
				{
					tokens.push_back(new_sptr<Token>(testOp, specMeaning->second, pos, buf.currentIndex(), opLen));
					buf.consume(opLen);
					pos.move(opLen);
					break;
				}

				--opLen;

			}

		}
		else
		{
			//if all else fails, skip it.
			buf.consume();
			pos.move();

		}

	}

	return tokens;
}
