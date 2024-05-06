
#include <algorithm>
#include <cctype>
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
	if (chr > 127)
	{
		return CharType::IDENTIFIER;
	}

	return asciiTypes[chr];
}

bool Tokenizer::findFloatFrac(out<std::stringstream> ss)
{
	bool isFloat = false;

	auto const isInt = LAMBDA(char chr)
	{
		return std::binary_search(DEC_INTS.begin(), DEC_INTS.end(), chr);
	};

	//Find the fractional component
	if (buf.hasRem(2) && buf.cur() == '.' && isInt(buf.peek(1)))
	{
		isFloat = true;

		ss << '.' << buf.peek(1);
		buf.consume(2);

		while (buf.hasCur())
		{
			auto decimal = buf.cur();

			if (isInt(decimal))
			{
				ss << decimal;
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
		auto exp = std::tolower(buf.cur());

		if (exp != 'e')
		{
			return isFloat;
		}

		size_t expStart = buf.offset();
		ss << exp;

		if (auto sign = buf.next(); sign == '+' || sign == '-')
		{
			ss << sign;
			buf.consume();
		}

		if (!isInt(buf.cur()))
		{
			buf.revertTo(expStart);
			return isFloat;
		}

		isFloat = true;

		do
		{
			ss << buf.cur();
			buf.consume();
		}
		while (buf.hasCur() && isInt(buf.cur()));

	}

	return isFloat;
}

size_t Tokenizer::findIntLiteral(out<TokenType> type, out<std::string> lit)
{
	auto validIntChars = &DEC_INTS;

	auto const isInt = LAMBDA(char chr)
	{
		return std::binary_search(validIntChars->begin(), validIntChars->end(), chr);
	};

	auto first = buf.cur();

	if (!isInt(first))
	{
		return 0;
	}

	size_t startIndex = buf.offset();
	bool isPrefixed = true;
	bool isFloat = false;

	std::stringstream ss;

	//find either a hex, binary, or octal integer
	if (first == '0' && buf.hasRem(3))
	{
		char litPrefix = std::tolower(buf.peek(1));

		switch (litPrefix)
		{
			case 'b': validIntChars = &BIN_INTS; break;
			case 'c': validIntChars = &OCT_INTS; break;
			case 'x': validIntChars = &HEX_INTS; break;
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
		
		if (isInt(digit))
		{
			//Make all lowercase hex digits uppercase, for everyone's sanity's sake
			ss << std::toupper(digit);
		}
		else if (digit != '_')
		{
			break;
		}

		buf.consume();

	}

	//find a decimal/float
	if (!isPrefixed)
	{
		isFloat = findFloatFrac(ss);
	}

	auto typeName = isFloat ? "fp32" : "int32";
	
	/*
	Caliburn outer-facing int literals have C-like suffixes. This bit of code finds one.

	Internally, we convert this to a more Rust-like suffix prefaced with an underscore.
	So 1f gets coverted to 1_fp32, 2.5f becomes 2.5_fp32, etc. This maintains outward-facing
	code aesthetics while giving the compiler itself the tools to very quickly parse a float
	*/
	if (buf.hasCur())
	{
		auto suffix = std::tolower(buf.cur());
		buf.consume();

		if (suffix == 'f')
		{
			typeName = "fp32";
		}
		else if (suffix == 'd')
		{
			typeName = "fp64";
		}
		else if (suffix == 'u')
		{
			if (buf.hasRem(2) && std::tolower(buf.peek(1)) == 'l')
			{
				buf.consume();
				typeName = "uint64";
			}
			else
			{
				typeName = "uint32";
			}

		}
		else if (suffix == 'l')
		{
			typeName = "int64";
		}
		else
		{
			buf.rewind();
		}
		
	}

	//Attach type information to the end of the literal
	//No, I don't want to expose this syntactically. Those literals look so ugly.
	ss << '_' << typeName;

	lit = ss.str();
	type = isFloat ? TokenType::LITERAL_FLOAT : TokenType::LITERAL_INT;

	size_t len = (buf.offset() - startIndex);

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
		const size_t start = buf.offset();
		const TextPos startPos = pos;
		const char current = buf.cur();

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
				doc->startLine(buf.offset() + 1);
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
			buf.consume();

			//skip to the end of the line, which will later invoke the whitespace code above
			while (buf.hasCur())
			{
				if (buf.cur() == '\n')
				{
					break;
				}

				if (buf.cur() != '\r')
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
				throw std::runtime_error((std::stringstream() << "Zero width identifier found at " << pos.toStr()).str());
			}

			if (wordLen > intLen)
			{
				tknContent = doc->text.substr(buf.offset(), wordLen);
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
					doc->startLine(buf.offset() + 1);
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
				throw std::runtime_error((std::stringstream() << "Unescaped string starts at " << delimTkn->pos.toStr()).str());
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
				const auto testOp = doc->text.substr(buf.offset(), opLen);

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

		tokens.push_back(new_sptr<Token>(tknContent, tknType, startPos, start, buf.offset()));

	}

	return tokens;
}
