
#pragma once

#include <vector>

namespace caliburn
{
	static const std::vector<std::string> KEYWORDS = {
		"as", "break",
		"case", "class", "const" "construct", "continue",
		"data", "def", "default", "descriptor", "destroy", "do", "dynamic",
		"extends",
		"false", "for",
		"if", "import", "in", "inputs",
		"let",
		"namespace", "new",
		"op", "override",
		"pass", "private", "protected", "public",
		"return",
		"shared", "switch",
		"true", "type",
		"uses",
		"while"
	};

	static constexpr auto OPERATORS = "!$%&*+-/<=>^|~";

	enum class TokenType : uint64_t
	{
		NONE,
		UNKNOWN,
		IDENTIFIER,
		LITERAL_INVALID,
		LITERAL_STR,
		LITERAL_INT,
		LITERAL_LONG,
		LITERAL_FLOAT,
		LITERAL_DOUBLE,
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
		MATH_OPERATOR,
		LOGIC_OPERATOR
	};

	enum class Operator
	{
		LOGIC_EQ, LOGIC_NEQ,
		LOGIC_GT, LOGIC_LT,
		LOGIC_GTE, LOGIC_LTE,
		LOGIC_AND, LOGIC_OR,

		ADD, SUB, MUL, DIV,
		MOD, POW,
		BIT_AND, BIT_OR, BIT_XOR,
		APPEND, INTDIV,

		//the following all need to be parsed manually
		//that is to say, you won't find them in the tables below
		//!, ~, -
		LOGIC_NOT, BIT_NOT, NEGATE,
		//|x|, x[n]
		ABS, ARRAY_ACCESS
	};

}

//this has to be a preprocessor directive =/
#define CALIBURN_CHAR_SYMBOL_TYPES {	\
	{';',	TokenType::END},			\
	{'{',	TokenType::START_SCOPE},	\
	{'}',	TokenType::END_SCOPE},		\
	{'[',	TokenType::START_BRACKET},	\
	{']',	TokenType::END_BRACKET},	\
	{'(',	TokenType::START_PAREN},	\
	{')',	TokenType::END_PAREN},		\
	{'.',	TokenType::PERIOD},			\
	{',',	TokenType::COMMA},			\
	{':',	TokenType::COLON}}

#define CALIBURN_STR_SYMBOL_TYPES { \
	{"true",	TokenType::LITERAL_BOOL},	\
	{"false",	TokenType::LITERAL_BOOL},	\
	{"&&",		TokenType::LOGIC_OPERATOR}, \
	{"||",		TokenType::LOGIC_OPERATOR}, \
	{"==",		TokenType::LOGIC_OPERATOR}, \
	{"!=",		TokenType::LOGIC_OPERATOR}, \
	{">",		TokenType::LOGIC_OPERATOR}, \
	{"<",		TokenType::LOGIC_OPERATOR}, \
	{">=",		TokenType::LOGIC_OPERATOR}, \
	{"<=",		TokenType::LOGIC_OPERATOR}}

//So here's why math ops are separate from logic ops:
//Math ops can use (op)= to set something, e.g. +=.
//Logic ops can't, mostly because I don't want to deal with parsing that.
#define CALIBURN_MATH_OPS {		\
	{"+",	Operator::ADD},		\
	{"++",	Operator::APPEND},	\
	{"-",	Operator::SUB},		\
	{"*",	Operator::MUL},		\
	{"/",	Operator::DIV},		\
	{"//",	Operator::INTDIV},	\
	{"%",	Operator::MOD},		\
	{"^",	Operator::POW},		\
	{"&",	Operator::BIT_AND},	\
	{"|",	Operator::BIT_OR},	\
	{"$",	Operator::BIT_XOR}}

#define CALIBURN_LOGIC_OPS {		\
	{"&&",	Operator::LOGIC_AND},	\
	{"||",	Operator::LOGIC_OR},	\
	{"==",	Operator::LOGIC_EQ},	\
	{"!=",	Operator::LOGIC_NEQ},	\
	{">",	Operator::LOGIC_GT},	\
	{"<",	Operator::LOGIC_LT},	\
	{">=",	Operator::LOGIC_GTE},	\
	{"<=",	Operator::LOGIC_LTE}}
