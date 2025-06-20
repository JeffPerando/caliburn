
#pragma once

#include <map>
#include <sstream>
#include <string_view>
#include <unordered_set>
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
	static const std::unordered_set<std::string_view> KEYWORDS = {
		//Array is a keyword not because it's used, but because it's an internal type
		"array",
		"break",
		"case", "class", "const", "continue",
		"def", "default", "delete", "discard", "do", "dynamic",
		"enum",
		"false", "for",
		"if", "import", "in", "is",
		"module",
		"new",
		"op", "override",
		"pass", "private", "public",
		"record", "return",
		"self", "shader", "shared", "sign", "strong", "struct", "switch",
		"this", "true", "type",
		"unreachable", "unsign", "uses",
		"var", "void",
		"while", "where", "wrapped"
	};

	static constexpr std::string_view OPERATOR_CHARS = "!$%&*+-/<=>^|~";

	/*
	These are all the ways a function statement can start.
	*/
	static const std::unordered_set<std::string_view> FN_STARTS = {
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
		std::string str;
		TokenType type = TokenType::UNKNOWN;
		TextPos pos;

		Token() = default;
		Token(in<Token> tkn) :
			str(tkn.str), type(tkn.type), pos(tkn.pos) {}
		Token(std::string s, TokenType t, TextPos p) :
			str(s), type(t), pos(p) {}

		Token operator=(in<Token> rhs)
		{
			str = rhs.str;
			type = rhs.type;
			pos = rhs.pos;

			return *this;
		}

		constexpr bool exists() const noexcept
		{
			return type != TokenType::UNKNOWN;
		}

		//TODO decide what looks good for a token
		void prettyPrint(std::stringstream ss) const
		{
			ss << str;
		}

	};

	/*
	A parsed object is simply an object which stores tokens for later interpretation.
	At minimum, they need to store the first and last tokens within their concept.
	This abstraction's main purpose is to allow for the easy printing of statements,
	expressions, values, etc. A good example is in error.h/cpp
	*/
	struct ParsedObject
	{
		virtual Token firstTkn() const noexcept = 0;

		virtual Token lastTkn() const noexcept = 0;

		virtual void prettyPrint(out<std::stringstream> ss) const = 0;

		virtual std::string prettyStr() const
		{
			std::stringstream ss;
			prettyPrint(ss);
			return ss.str();
		}

	};

	enum class Operator : uint32_t
	{
		NONE,

		ADD, SUB, MUL, DIV, INTDIV,
		MOD, POW,

		BIT_AND, BIT_OR, BIT_XOR,
		SHIFT_LEFT, SHIFT_RIGHT,

		COMP_EQ, COMP_NEQ,
		COMP_GT, COMP_LT,
		COMP_GTE, COMP_LTE,
		LOGIC_AND, LOGIC_OR,

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
	This enables for an arbitrary string to have a specific token type
	*/
	static const HashMap<std::string_view, TokenType> TOKEN_TYPE_OVERRIDES = {
		{"=",		TokenType::SETTER},
		{"(",		TokenType::START_PAREN},
		{")",		TokenType::END_PAREN},
		{",",		TokenType::COMMA},
		{".",		TokenType::PERIOD},
		{":",		TokenType::COLON},
		{";",		TokenType::END},
		{"[",		TokenType::START_BRACKET},
		{"]",		TokenType::END_BRACKET},
		{"{",		TokenType::START_SCOPE},
		{"}",		TokenType::END_SCOPE},
		{"true",	TokenType::LITERAL_BOOL},
		{"false",	TokenType::LITERAL_BOOL},
		{"->",		TokenType::ARROW},
		{"=>",		TokenType::ARROW}
	};

	/*
	Used by the tokenizer to discern valid operator tokens of length 2 or more. Single-char ops are always valid.
	*/
	static const std::unordered_set<std::string_view> LONG_OPS = {
		"++", "//", "&&", "||", "<<", ">>", "==", "!=", ">=", "<=", "->", "=>"
	};

	/*
	Unary ops have a higher precedent than infix ops, hence this separate map
	*/
	static const HashMap<std::string_view, Operator> UNARY_OPS = {
		{"|",	Operator::ABS},
		{"-",	Operator::NEG},
		{"~",	Operator::BIT_NEG},
		{"!",	Operator::BOOL_NOT}
	};

	static const HashMap<std::string_view, Operator> INFIX_OPS = {
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

		{"&&",	Operator::LOGIC_AND},
		{"||",	Operator::LOGIC_OR},
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
	static const std::map<Operator, std::string_view> INFIX_OPS_STR = {
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

		{Operator::LOGIC_AND,	"&&"},
		{Operator::LOGIC_OR,	"||"},
		{Operator::COMP_EQ,		"=="},
		{Operator::COMP_NEQ,	"!="},
		{Operator::COMP_GT,		">"},
		{Operator::COMP_LT,		"<"},
		{Operator::COMP_GTE,	">="},
		{Operator::COMP_LTE,	"<="},

		{Operator::APPEND,		"++"}
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

		{Operator::LOGIC_AND,	OpCategory::LOGICAL},
		{Operator::LOGIC_OR,	OpCategory::LOGICAL},
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
		{Operator::LOGIC_AND,	10},
		{Operator::LOGIC_OR,	9},

		{Operator::COMP_EQ,		8},
		{Operator::COMP_NEQ,	8},
		{Operator::COMP_GT,		8},
		{Operator::COMP_LT,		8},
		{Operator::COMP_GTE,	8},
		{Operator::COMP_LTE,	8},

		{Operator::SHIFT_LEFT,	7},
		{Operator::SHIFT_RIGHT,	7},
		{Operator::BIT_AND,		6},
		{Operator::BIT_OR,		5},
		{Operator::BIT_XOR,		5},

		{Operator::APPEND,		4},
		{Operator::POW,			3},
		{Operator::MUL,			2},
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
