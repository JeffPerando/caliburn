
#pragma once

#include <map>
#include <sstream>
#include <string_view>
#include <vector>

#include "basic.h"
#include "strhelp.h"

namespace caliburn
{
	/*
	These are effectively lists of the valid characters within a particular type of integer literal.

	Underscores are always valid and can be used to make ints more readable
	*/

	static const std::string_view BIN_INTS = "01";
	static const std::string_view OCT_INTS = "01234567";
	static const std::string_view DEC_INTS = "0123456789";
	static const std::string_view HEX_INTS = "0123456789ABCDEFabcdef";
	
	/*
	This is the complete list of Caliburn's reserved keywords.

	Some of these are also types. These are not valid for users to use; Rather,
	they are reserved to enable for internal use.
	*/
	static const std::vector<std::string_view> KEYWORDS = {
		"array", "as",
		"break",
		"case", "class", "const", "construct", "continue",
		"def", "default", "delete", "destruct", "discard", "do", "dynamic",
		"enum", "extends",
		"false", "for",
		"if", "import", "in", "is",
		"module",
		"new",
		"op", "override",
		"pass", "private", "public",
		"record", "return",
		"shader", "shared", "sign", "strong", "struct", "switch",
		"this", "true", "type",
		"unreachable", "unsign", "uses",
		"var", "void",
		"while", "where", "wrapped"
	};

	/*
	This is every single character which is also an operator.

	I'm not sure why it's a string.
	*/
	static constexpr std::string_view OPERATOR_CHARS = "!$%&*+-/<=>^|~";

	/*
	These are all the ways a function statement can start.
	*/
	static const std::vector<std::string_view> FN_STARTS = {
		"construct", "def", "destruct", "op", "override"
	};

	/*
	TokenType is a shorthand way to describe the meaning of a token's contents.

	Generally, the parser will look at a token's type to determine if it's valid for a given purpose.

	This also includes things which typically only refer to one particular character. This is to prevent
	a string comparison, and to enable for easier syntax changes should the need arise.
	*/
	enum class TokenType : uint8_t
	{
		UNKNOWN,
		WHITESPACE,
		IDENTIFIER,
		LITERAL_STR,
		LITERAL_INT,
		LITERAL_FLOAT,
		LITERAL_BOOL,
		KEYWORD,
		END,
		START_SCOPE,
		END_SCOPE,
		START_BRACKET,
		END_BRACKET,
		START_PAREN,
		END_PAREN,
		PERIOD,
		COMMA,
		COLON,
		SETTER,
		OPERATOR,
		ARROW
	};

	/*
	A token represents a string with a specific meaning. Tokens also store where they were in
	the original text.
	*/
	struct Token
	{
		const std::string str;
		const TokenType type;
		const TextPos pos;

		/*
		These represent the start and end of this token within the file itself.
		Since a file is just a string, we can index directly into it.
		ALSO: textEnd would be redundant, EXCEPT some tokens can be shortened, e.g. int literals
		So, since it's not a 1:1 text to token translation, we need to include the start AND end.
		*/
		const uint64_t textStart, textEnd;

		Token(std::string t,
			TokenType id = TokenType::IDENTIFIER, TextPos p = TextPos(),
			uint64_t s = 0, uint64_t off = 0) :
			str(t), type(id), pos(p), textStart(s), textEnd(s + off) {}

		Token(char c, TokenType id = TokenType::IDENTIFIER,
			TextPos p = TextPos(), uint64_t off = 0) :
			str(std::string(1, c)), type(id), pos(p), textStart(off), textEnd(off + 1) {}

		//TODO decide what looks good for a token
		void prettyPrint(std::stringstream ss)
		{
			ss << str;
		}

	};

	/*
	This is every operator, both unary and infix, in Caliburn.
	*/
	enum class Operator : uint32_t
	{
		UNKNOWN,

		ADD, SUB, MUL, DIV, INTDIV,
		MOD, POW,

		BIT_AND, BIT_OR, BIT_XOR,
		SHIFT_LEFT, SHIFT_RIGHT,

		COMP_EQ, COMP_NEQ,
		COMP_GT, COMP_LT,
		COMP_GTE, COMP_LTE,
		AND, OR,

		APPEND,

		//Unary ops
		ABS, NEG, BIT_NEG, BOOL_NOT

	};

	/*
	Operators have categories; This mainly determines things like output type, how the
	operator is parsed, capabilities, etc.
	*/
	enum class OpCategory : uint32_t
	{
		ARITHMETIC,
		BITWISE,
		LOGICAL,
		UNARY,
		MISC
	};

	/*
	A return mode is a statement which ends a scope.
	*/
	enum class ReturnMode
	{
		NONE,
		RETURN,
		CONTINUE,
		BREAK,
		PASS,
		UNREACHABLE,
		DISCARD
	};

	/*
	A parsed object is simply an object which stores tokens for later interpretation.

	At minimum, they need to store the first and last tokens within their concept.

	This abstraction's main purpose is to allow for the easy printing of statements,
	expressions, values, etc. A good example is in error.h/cpp
	*/
	struct ParsedObject
	{
		virtual sptr<Token> firstTkn() const = 0;

		virtual sptr<Token> lastTkn() const = 0;

		virtual size_t totalParsedLen() const
		{
			auto first = firstTkn();
			auto last = lastTkn();

			if (first == nullptr || last == nullptr)
			{
				return 0;
			}

			return last->textEnd - first->textStart;
		}

		virtual void prettyPrint(ref<std::stringstream> ss) const = 0;

		virtual std::string prettyStr() const
		{
			std::stringstream ss;

			prettyPrint(ss);

			return ss.str();
		}

	};

	/*
	Individual characters can have their own token types
	*/
	static const HashMap<char, TokenType> CHAR_TOKEN_TYPES = {
		{'(',	TokenType::START_PAREN},
		{')',	TokenType::END_PAREN},
		{',',	TokenType::COMMA},
		{'.',	TokenType::PERIOD},
		{':',	TokenType::COLON},
		{';',	TokenType::END},
		{'[',	TokenType::START_BRACKET},
		{']',	TokenType::END_BRACKET},
		{'{',	TokenType::START_SCOPE},
		{'}',	TokenType::END_SCOPE}
	};

	/*
	Some keywords have their own token types as well
	*/
	static const HashMap<std::string, TokenType> STR_TOKEN_TYPES = {
		{"true",	TokenType::LITERAL_BOOL},
		{"false",	TokenType::LITERAL_BOOL}
	};

	/*
	This overrides the token type of a string which only contains operator chars
	*/
	static const std::map<std::string, TokenType> SPECIAL_OPS = {
		{"=",	TokenType::SETTER},
		{"->",	TokenType::ARROW},
		{"=>",	TokenType::ARROW}
	};

	/*
	Unary ops have a higher precedent than infix ops, hence this separate map
	*/
	static const HashMap<std::string, Operator> UNARY_OPS = {
		{"|",	Operator::ABS},
		{"-",	Operator::NEG},
		{"~",	Operator::BIT_NEG},
		{"!",	Operator::BOOL_NOT}
	};

	static const HashMap<std::string, Operator> INFIX_OPS = {
		{"+",	Operator::ADD},
		{"-",	Operator::SUB},
		{"*",	Operator::MUL},
		{"/",	Operator::DIV},
		{"//",	Operator::INTDIV},
		{"%",	Operator::MOD},
		{"^",	Operator::POW},

		{"&",	Operator::BIT_AND},
		{"|",	Operator::BIT_OR},
		{"$",	Operator::BIT_XOR},
		{"<<",	Operator::SHIFT_LEFT},
		{">>",	Operator::SHIFT_RIGHT},

		{"&&",	Operator::AND},
		{"||",	Operator::OR},
		{"==",	Operator::COMP_EQ},
		{"!=",	Operator::COMP_NEQ},
		{">",	Operator::COMP_GT},
		{"<",	Operator::COMP_LT},
		{">=",	Operator::COMP_GTE},
		{"<=",	Operator::COMP_LTE},

		{"++",	Operator::APPEND}
	};

	/*
	A map which goes in reverse of INFIX_OPS. Used for debugging, mainly.
	*/
	static const std::map<Operator, std::string> INFIX_OPS_STR = {
		{Operator::ADD,			"+"},
		{Operator::SUB,			"-"},
		{Operator::MUL,			"*"},
		{Operator::DIV,			"/"},
		{Operator::INTDIV,		"//"},
		{Operator::MOD,			"%"},
		{Operator::POW,			"^"},

		{Operator::BIT_AND,		"&"},
		{Operator::BIT_OR,		"|"},
		{Operator::BIT_XOR,		"$"},
		{Operator::SHIFT_LEFT,	"<<"},
		{Operator::SHIFT_RIGHT,	">>"},

		{Operator::AND,			"&&"},
		{Operator::OR,			"||"},
		{Operator::COMP_EQ,		"=="},
		{Operator::COMP_NEQ,	"!="},
		{Operator::COMP_GT,		">"},
		{Operator::COMP_LT,		"<"},
		{Operator::COMP_GTE,	">="},
		{Operator::COMP_LTE,	"<="},

		{Operator::APPEND,		"++"}
	};

	static const HashMap<std::string, ReturnMode> RETURN_MODES = {
		{"return",		ReturnMode::RETURN},
		{"continue",	ReturnMode::CONTINUE},
		{"break",		ReturnMode::BREAK},
		{"pass",		ReturnMode::PASS},
		{"unreachable",	ReturnMode::UNREACHABLE},
		{"discard",		ReturnMode::DISCARD}
	};

	static const std::map<Operator, OpCategory> OP_CATEGORIES = {
		{Operator::ADD,			OpCategory::ARITHMETIC},
		{Operator::SUB,			OpCategory::ARITHMETIC},
		{Operator::MUL,			OpCategory::ARITHMETIC},
		{Operator::DIV,			OpCategory::ARITHMETIC},
		{Operator::INTDIV,		OpCategory::ARITHMETIC},
		{Operator::MOD,			OpCategory::ARITHMETIC},
		{Operator::POW,			OpCategory::ARITHMETIC},

		{Operator::BIT_AND,		OpCategory::BITWISE},
		{Operator::BIT_OR,		OpCategory::BITWISE},
		{Operator::BIT_XOR,		OpCategory::BITWISE},
		{Operator::SHIFT_LEFT,	OpCategory::BITWISE},
		{Operator::SHIFT_RIGHT,	OpCategory::BITWISE},

		{Operator::AND,			OpCategory::LOGICAL},
		{Operator::OR,			OpCategory::LOGICAL},
		{Operator::COMP_EQ,		OpCategory::LOGICAL},
		{Operator::COMP_NEQ,	OpCategory::LOGICAL},
		{Operator::COMP_GT,		OpCategory::LOGICAL},
		{Operator::COMP_LT,		OpCategory::LOGICAL},
		{Operator::COMP_GTE,	OpCategory::LOGICAL},
		{Operator::COMP_LTE,	OpCategory::LOGICAL},

		{Operator::ABS,			OpCategory::UNARY},
		{Operator::NEG,			OpCategory::UNARY},
		{Operator::BIT_NEG,		OpCategory::UNARY},
		{Operator::BOOL_NOT,	OpCategory::UNARY},

		{Operator::APPEND,		OpCategory::MISC},
	};

	static const std::map<Operator, uint32_t> OP_PRECEDENCE = {
		{Operator::AND,			10},
		{Operator::OR,			10},

		{Operator::COMP_EQ,		9},
		{Operator::COMP_NEQ,	9},
		{Operator::COMP_GT,		9},
		{Operator::COMP_LT,		9},
		{Operator::COMP_GTE,	9},
		{Operator::COMP_LTE,	9},

		{Operator::SHIFT_LEFT,	8},
		{Operator::SHIFT_RIGHT,	8},
		{Operator::BIT_AND,		7},
		{Operator::BIT_OR,		6},
		{Operator::BIT_XOR,		6},

		{Operator::APPEND,		5},
		{Operator::POW,			4},
		{Operator::MUL,			3},
		{Operator::DIV,			2},
		{Operator::INTDIV,		2},
		{Operator::MOD,			2},
		{Operator::ADD,			1},
		{Operator::SUB,			1}
	};

	//Since generics share a symbol with certain operators, these constants exist to make parsing code look cleaner and less insane.

	static constexpr std::string_view GENERIC_START = "<";
	static constexpr std::string_view GENERIC_END = ">";

}
