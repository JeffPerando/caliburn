
import argparse
import re

import syntax

CALIBURN_DEBUG_MODE = True

def isIntStart(chr):
    return (chr >= '0' and chr <= '9')

def isInt(chr):
    return isIntStart(chr) or (".f_xE+-".find(chr) != -1) or (chr >= 'A' and chr <= 'F') or (chr >= 'a' and chr <= 'f')

def isIdentifier(chr):
    return (chr >= 'a' and chr <= 'z') or (chr >= 'A' and chr <= 'Z') or isIntStart(chr) or (chr == '_')

def isOperator(chr):
    return "=+-*/<>%^&|$!~".count(chr) > 0

def parseOp(txt, cur):
    tokenLen = 1
    while isOperator(txt[cur + tokenLen]):
        tokenLen += 1
    return tokenLen

def parseStr(txt, cur, delim):
    tokenLen = 1
    while txt[cur + tokenLen] != delim and txt[cur + tokenLen - 1] != "\\":
        tokenLen += 1
    tokenLen += 1
    return tokenLen

def parseInt(txt, cur):
    tokenLen = 1
    while isInt(txt[cur + tokenLen]):
        tokenLen += 1
    return tokenLen

def parseIdentifier(txt, cur) -> int:
    tokenLen = 1
    while isIdentifier(txt[cur + tokenLen]):
        tokenLen += 1
    return tokenLen

def tokenize(txt):
    # Only used to tell humans where the token was in the source file
    line = 1
    col = 1

    cur = 0
    tokens = []
    miscTokenValues = {
        'true': syntax.CALIBURN_V_LITERAL_BOOL,
        'false': syntax.CALIBURN_V_LITERAL_BOOL,
        ';': syntax.CALIBURN_V_END,
        '{': syntax.CALIBURN_V_START_SCOPE,
        '}': syntax.CALIBURN_V_END_SCOPE,
        '[': syntax.CALIBURN_V_START_BRACKET,
        ']': syntax.CALIBURN_V_END_BRACKET,
        '(': syntax.CALIBURN_V_START_PAREN,
        ')': syntax.CALIBURN_V_END_PAREN,
        '<': syntax.CALIBURN_V_LT_SIGN,
        '>': syntax.CALIBURN_V_GT_SIGN,
        '.': syntax.CALIBURN_V_PERIOD,
        ',': syntax.CALIBURN_V_COMMA,
        ':': syntax.CALIBURN_V_COLON,
        '|': syntax.CALIBURN_V_LINE}
    
    for kw in syntax.CALIBURN_KEYWORDS:
        miscTokenValues[kw] = syntax.CALIBURN_V_KEYWORD

    while cur < len(txt):
        # Avoid whitespace (note to future self: use the string " \t\n\r\f\v" when implementing this in C++)
        if txt[cur].isspace():
            if txt[cur] == '\n':
                line += 1
                col = 1
            else:
                col += 1
            cur += 1
            continue
        
        # Avoid comments
        if txt[cur] == "#":
            # Avoid multi-line comments
            if txt[cur+1:cur+3] == "##":
                cur += 3
                while txt[cur:cur+3] != "###":
                    if txt[cur] == '\n':
                        line += 1
                        col = 1
                    cur += 1
                    col += 1
                col += 3
                cur += 3
            else:
                while txt[cur] != '\n':
                    cur += 1
                # Current will end up being on a newline; need to increment once more
                cur += 1
                line += 1
                col = 1
            continue
        
        tokenID = miscTokenValues.get(txt[cur], 0)

        if tokenID and (not isOperator(txt[cur]) or not isOperator(txt[cur + 1])):
            tokens.append((txt[cur], 1, line, col))
            cur += 1
            continue
        
        tokenLen = 1
        # Identifiers can include integers, so check for an int first
        # ALSO this part's kinda weird so let me run down what's going on:
        # If we find a [0-9], then there's three possibilities for what's next:
        # 1. It's a number or a character associated thereof, like a period (i.e. 0.1f)
        # 2. It's a part of the punctuation, like a comma, parentheses, etc.
        # 3. It's some other character
        # It's possible a class could start with a number, so we look ahead one.
        # If the character is a #1, then that's fine.
        # If it's punctuation, then oh well, the lexer will deal with it later.
        # If it's something else, then we need to know.
        # SO, if the next char is an identifier, then this branch fails and it finds the identifier.
        # If not, then we don't care anyway and go through with getting an int.
        # Note: This doesn't work for identifiers that start with multiple digits (like "404ErrorNotFound")
        # I need to fix that...
        # One more thing: if we hit an EOF, then that's a colossal oof and the file is probably corrupt.
        
        if isIntStart(txt[cur]) and (isInt(txt[cur+1]) or not isIdentifier(txt[cur+1])):
            tokenLen = parseInt(txt, cur)
            if isIdentifier(txt[cur + tokenLen + 1]):
                tokenLen = parseIdentifier(txt, cur)
                tokenID = syntax.CALIBURN_V_IDENTIFIER
            else:
                tokenID = syntax.CALIBURN_V_LITERAL_INT
        elif isIdentifier(txt[cur]):
            tokenLen = parseIdentifier(txt, cur)
            tokenID = syntax.CALIBURN_V_IDENTIFIER
        elif isOperator(txt[cur]):
            tokenLen = parseOp(txt, cur)
            tokenID = syntax.CALIBURN_V_OPERATOR
        elif txt[cur] == "\"" or txt[cur] == "\'":
            tokenLen = parseStr(txt, cur, txt[cur])
            tokenID = syntax.CALIBURN_V_LITERAL_STR

        tokenStr = txt[cur:cur+tokenLen]
        if tokenID == syntax.CALIBURN_V_IDENTIFIER and miscTokenValues.get(tokenStr, None):
            tokenID = syntax.CALIBURN_V_KEYWORD

        tokens.append((tokenStr, tokenID, line, col))
        cur += tokenLen
        col += tokenLen

    return tokens

def readFile(path):
    text = ""
    with open(path) as f:
        text = f.read()
    return text

def main(path="test.txt"):
    src = readFile(path)
    tokens = tokenize(src)
    for tk in tokens:
        print(f"{tk}")

if __name__ == "__main__":
    if CALIBURN_DEBUG_MODE:
        main()
    else:
        parser = argparse.ArgumentParser(description='Prototype compiler for Caliburn')
        parser.add_argument('filepath', metavar='source', type=str,
                            help='the path or name of the source file to be compiled', required = False)

        args = parser.parse_args()
        main(args.filepath)