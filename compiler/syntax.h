
#pragma once

#include <array>

namespace caliburn
{
	enum class TokenType : int64_t
	{
		NONE,
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
		OPERATOR,
		BOOL_OPERATOR
	};

	static const std::vector<std::string> KEYWORDS = {
		"break",
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
		"uses", "using",
		"while"
	};

	static constexpr auto WHITESPACE = " \t\n\r\f\v";
	static constexpr auto OPERATORS = "!$%&*+-/<=>^|~";

}

//this has to be a preprocessor directive =/
#define CALIBURN_TOKEN_VALUES { \
	{';', caliburn::TokenType::END},\
	{'{', caliburn::TokenType::START_SCOPE},\
	{'}', caliburn::TokenType::END_SCOPE},\
	{'[', caliburn::TokenType::START_BRACKET},\
	{']', caliburn::TokenType::END_BRACKET},\
	{'(', caliburn::TokenType::START_PAREN},\
	{')', caliburn::TokenType::END_PAREN},\
	{'.', caliburn::TokenType::PERIOD},\
	{',', caliburn::TokenType::COMMA},\
	{':', caliburn::TokenType::COLON}}

#define CALIBURN_SPECIAL_OPERATORS { \
	{"<", caliburn::TokenType::BOOL_OPERATOR}, \
	{">", caliburn::TokenType::BOOL_OPERATOR}, \
	{"==", caliburn::TokenType::BOOL_OPERATOR}, \
	{"!=", caliburn::TokenType::BOOL_OPERATOR}, \
	{"<=", caliburn::TokenType::BOOL_OPERATOR},\
	{">=", caliburn::TokenType::BOOL_OPERATOR}}
