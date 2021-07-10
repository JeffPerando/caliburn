
#pragma once

#define CALIBURN_T_IDENTIFIER		(1)
#define CALIBURN_T_LITERAL_STR		(2)
#define CALIBURN_T_LITERAL_INT		(3)
#define CALIBURN_T_LITERAL_LONG		(4)
#define CALIBURN_T_LITERAL_FLOAT	(5)
#define CALIBURN_T_LITERAL_DOUBLE	(6)
#define CALIBURN_T_LITERAL_BOOL		(7)
#define CALIBURN_T_KEYWORD			(8)
#define CALIBURN_T_END				(9)
#define CALIBURN_T_START_SCOPE		(10)
#define CALIBURN_T_END_SCOPE		(11)
#define CALIBURN_T_START_BRACKET	(12)
#define CALIBURN_T_END_BRACKET		(13)
#define CALIBURN_T_START_PAREN		(14)
#define CALIBURN_T_END_PAREN		(15)
#define CALIBURN_T_LT_SIGN			(16)
#define CALIBURN_T_GT_SIGN			(17)
#define CALIBURN_T_PERIOD			(18)
#define CALIBURN_T_COMMA			(19)
#define CALIBURN_T_COLON			(20)
#define CALIBURN_T_LINE				(21)
#define CALIBURN_T_OPERATOR			(22)

#define CALIBURN_KEYWORDS { \
	"import", "using", "type", "namespace", "data", "class", "descriptor", "inputs", "def", \
	"override", "op", "construct", "destroy", "extends", \
	"let", "new", "public", "protected", "private", "shared", "const", "dynamic", \
	"if", "for", "in", "while", "do", "switch", "case", "default", "pass", "continue", "break", "return"}

#define CALIBURN_WHITESPACE (std::string(" \t\n\r\f\v"))

#define CALIBURN_TOKEN_VALUES { \
{"true", CALIBURN_T_LITERAL_BOOL},\
{"false", CALIBURN_T_LITERAL_BOOL},\
{";", CALIBURN_T_END},\
{"{", CALIBURN_T_START_SCOPE},\
{"}", CALIBURN_T_END_SCOPE},\
{"[", CALIBURN_T_START_BRACKET},\
{"]", CALIBURN_T_END_BRACKET},\
{"(", CALIBURN_T_START_PAREN},\
{")", CALIBURN_T_END_PAREN},\
{"<", CALIBURN_T_LT_SIGN},\
{">", CALIBURN_T_GT_SIGN},\
{".", CALIBURN_T_PERIOD},\
{",", CALIBURN_T_COMMA},\
{":", CALIBURN_T_COLON},\
{"|", CALIBURN_T_LINE}}
