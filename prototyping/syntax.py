
import re

CALIBURN_WORDS_AND_SYMBOLS  = re.compile("\".*\"|\'.*\'|[a-zA-Z0-9_]+|[+\-*\/.,:;\(\)\{\}\[\]<>=!]")

CALIBURN_KEYWORDS           = [
    "import", "using", "type", "namespace", "data", "class", "descriptor", "inputs", "def",
    "override", "op", "let",
    "construct", "destroy", "new",
    "public", "protected", "private", "shared", "const", "dynamic",
    "if", "for", "in", "while", "do", "switch", "case", "default", "pass", "continue", "break", "return",
    "true", "false"
    ]

CALIBURN_BUILTIN_FNS        = []

CALIBURN_INT_LITERAL_RE     = re.compile("[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?f?")
CALIBURN_HEX_LITERAL_RE     = re.compile("0x([0-9a-fA-F]|_)+")
CALIBURN_BIN_LITERAL_RE     = re.compile("0b[0|1|_]+")
CALIBURN_OCT_LITERAL_RE     = re.compile("0c([0-7]|_)+")

CALIBURN_MATH_OPS_RE        = re.compile("(\+\+)|(\/\/)|[\+\-*\/%^&|$]")
CALIBURN_OTHER_OPS_RE       = re.compile("[<=>!]=?|(&&)|(\|\|)")

CALIBURN_FLOAT_TYPE_RE      = re.compile("float(16|32|64|128)?")
CALIBURN_INT_TYPE_RE        = re.compile("u?int(8|16|32|64|128|256|512)?")
CALIBURN_VECTOR_TYPE_RE     = re.compile("vec(2|3|4)")
CALIBURN_MATRIX_TYPE_RE     = re.compile("mat[2-4](x[2-4])?")
CALIBURN_TEXTURE_TYPE_RE    = re.compile("tex(ture)?([1-3]D|Cube)")

CALIBURN_GENERIC_TYPES      = ["array", "buffer", "pointer", "ptr", "atomic"]

# These types are completely optional and may be delayed for future versions of the language
CALIBURN_CHAR_TYPE_RE       = re.compile("char(8|32)?")
CALIBURN_STRING_TYPE_RE     = re.compile("str(ing)?(8|32)?|unicode")
CALIBURN_BITSET_TYPE_RE     = re.compile("bitset(8|16|32|64|128|256)?")
CALIBURN_MISC_TYPES         = ["void", "bool", "byte", "bfloat16"]

CALIBURN_V_IDENTIFIER       = 1     # An identifier is a string of characters not escaped by " or '.
CALIBURN_V_LITERAL          = 2     # A literal is either a number, string, or a boolean.
CALIBURN_V_KEYWORD          = 3     # Denotes an existing keyword
CALIBURN_V_END              = 4     # Denotes a ;
CALIBURN_V_START_SCOPE      = 5     # Denotes a {
CALIBURN_V_END_SCOPE        = 6     # Denotes a }
CALIBURN_V_START_BRACKET    = 7     # Denotes a [
CALIBURN_V_END_BRACKET      = 8     # Denotes a ]
CALIBURN_V_START_PAREN      = 9     # Denotes a (
CALIBURN_V_END_PAREN        = 10    # Denotes a )
CALIBURN_V_LT_SIGN          = 11    # Denotes a <
CALIBURN_V_GT_SIGN          = 12    # Denotes a >
CALIBURN_V_OPERATOR         = 13    # Denotes an operator (=+-*/%^&|$!~)