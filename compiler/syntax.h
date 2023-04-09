
#pragma once

#include <map>
#include <vector>

namespace caliburn
{
	static const std::vector<std::string_view> KEYWORDS = {
		"as",
		"break",
		"case", "class", "const", "construct", "continue",
		"def", "default", "delete", "destruct", "discard", "do", "dynamic",
		"enum", "extends",
		"false", "for",
		"if", "import", "in", "is",
		"make", "module",
		"new",
		"op", "override",
		"pass", "private", "protected", "public",
		"record", "return",
		"shader", "shared", "sign", "strong", "struct", "switch",
		"this", "true", "type",
		"unreachable", "unsign", "uses",
		"var",
		"while", "where", "wrapped"
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
		OPERATOR,
		ARROW
	};

	struct Token
	{
		const std::string str;
		const TokenType type;
		const uint64_t line, column;
		
		/*
		These represent the start and end of this token within the file itself.
		Since a file is just a string, we can index directly into it.
		ALSO: The end field would be redundant, EXCEPT some tokens can be shortened, e.g. int literals
		So, since it's not a 1:1 text to token translation, we need to include the start AND end.
		*/
		const uint64_t textStart, textEnd;

		Token(std::string t,
			TokenType id = TokenType::IDENTIFIER,
			uint64_t l = 0, uint64_t c = 0,
			uint64_t s = 0, uint64_t off = 0) :
			str(t), type(id), line(l), column(c), textStart(s), textEnd(s + off) {}

	};

	enum class Operator : uint32_t
	{
		UNKNOWN,
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
		//|x|
		ABS
	};
	
	struct ParsedObject
	{
		virtual Token* firstTkn() const = 0;

		virtual Token* lastTkn() const = 0;

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

	static const std::map<std::string, TokenType> specialOps = {
		{"->",	TokenType::ARROW},
		{"=>",	TokenType::ARROW}
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
