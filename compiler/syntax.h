
#pragma once

#include <map>
#include <vector>

namespace caliburn
{
	static const std::vector<std::string> KEYWORDS = {
		"as",
		"break",
		"case", "class", "const" "construct", "continue",
		"data", "def", "default", "descriptor", "destruct", "do", "dynamic",
		"extends",
		"false", "for",
		"if", "import", "in", "inputs",
		"make", "module",
		"new",
		"op", "out", "override",
		"pass", "private", "protected", "public",
		"return",
		"shader", "shared", "struct", "switch",
		"true", "type",
		"uses",
		"var",
		"while"
	};

	static constexpr std::string_view OPERATORS = "!$%&*+-/<=>^|~";

	enum class TokenType : uint64_t
	{
		UNKNOWN,
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
		OPERATOR
	};

	struct Token
	{
		std::string str;
		TokenType type = TokenType::UNKNOWN;
		uint64_t line, column;

		Token(std::string t) : Token(t, TokenType::IDENTIFIER) {}
		Token(std::string t, TokenType id) : Token(t, id, 0, 0) {}

		//the fact that this has to exist is bothersome
		Token(std::string t, TokenType id, uint64_t l, uint64_t c) :
			str(t), type(id), line(l), column(c) {}

	};

	enum class Operator
	{
		COMP_EQ, COMP_NEQ,
		COMP_GT, COMP_LT,
		COMP_GTE, COMP_LTE,
		COND_AND, COND_OR,

		ADD, SUB, MUL, DIV,
		MOD, POW,
		BIT_AND, BIT_OR, BIT_XOR,
		SHIFT_LEFT, SHIFT_RIGHT,
		APPEND, INTDIV,

		//the following all need to be parsed manually
		//that is to say, you won't find them in the tables below
		//!, ~, -
		BOOL_NOT, BIT_NOT, NEGATE,
		//|x|, x[n]
		ABS, ARRAY_ACCESS
	};

	static const std::map<char, TokenType> charTokenTypes = {
		{';',	TokenType::END},
		{'{',	TokenType::START_SCOPE},
		{'}',	TokenType::END_SCOPE},
		{'[',	TokenType::START_BRACKET},
		{']',	TokenType::END_BRACKET},
		{'(',	TokenType::START_PAREN},
		{')',	TokenType::END_PAREN},
		{'.',	TokenType::PERIOD},
		{',',	TokenType::COMMA},
		{':',	TokenType::COLON}
	};

	static const std::map<std::string, TokenType> strTokenTypes = {
		{"true",	TokenType::LITERAL_BOOL},
		{"false",	TokenType::LITERAL_BOOL},
	};

	//So here's why math ops are separate from logic ops:
	//Math ops can use (op)= to set something, e.g. +=.
	//Logic ops can't, mostly because I don't want to deal with parsing that.

	static const std::map<std::string, Operator> mathOps = {
		{"+",	Operator::ADD},
		{"++",	Operator::APPEND},
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
		{">>",	Operator::SHIFT_RIGHT}
	};
	
	static const std::map<std::string, Operator> logicOps = {
		{"&&",	Operator::COND_AND},
		{"||",	Operator::COND_OR},
		{"==",	Operator::COMP_EQ},
		{"!=",	Operator::COMP_NEQ},
		{">",	Operator::COMP_GT},
		{"<",	Operator::COMP_LT},
		{">=",	Operator::COMP_GTE},
		{"<=",	Operator::COMP_LTE}
	};

	static constexpr std::string_view GENERIC_START = "<";
	static constexpr std::string_view GENERIC_END = ">";

}
