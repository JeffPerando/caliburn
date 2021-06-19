
import syntax

from enum import Enum
import typing

Tokens = list[tuple[str, int]]

class DeclarationType(Enum):
    IMPORT = 1,
    USING = 2,
    TYPEDEF = 3,
    NAMESPACE = 4,
    STRUCT = 5,
    CLASS = 6,
    DESCRIPTOR = 7,
    INPUTS = 8,
    FUNCTION = 9,
    VARIABLE = 10,
    CONTROL_FLOW = 11

class ParsedDecl:
    def __init__(self, decltype: DeclarationType):
        self.type = decltype

class StatementType(Enum):
    FUNCTION = 1,
    CONSTRUCTOR = 2,
    DESTRUCTOR = 3,
    VARIBLE = 4,
    SETTER = 5,
    CONTROL = 6,
    SCOPE = 7

class ControlFlow(Enum):
    IF = 1,
    FOR = 2,
    WHILE = 3,
    DOWHILE = 4,
    SWITCH = 5,
    BREAK = 6,
    CONTINUE = 7,
    PASS = 8,
    RETURN = 9

def parse(tkns: Tokens):
    pass

def parseDecl(tkns, cur):
    pass