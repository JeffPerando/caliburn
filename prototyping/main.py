
import argparse
import re
import syntax

CALIBURN_DEBUG_MODE = True

class Token:
    def __init__(self, str, tid):
       self.chars = str
       self.id = tid

def regexFix(pattern, txt):
    found = pattern.findall(txt)
    fixedSet = []
    for tp in found:
        for x in tp:
            if x is None:
                continue
            if len(x) == 0:
                continue
            fixedSet.append(x)
            break
    return fixedSet

# Find and substring: Finds a token and then returns a tuple containing:
# 1. A substring from the start of the search to the token found (includes the token if possible)
# 2. The index where the token was found plus the length
def findAndSubstr(txt, tkn, start, checkEsc = False):
    next = txt.find(tkn, start + len(tkn))
    if checkEsc:
        while txt[next - 1] == "\\":
            next = txt.find(tkn, next + len(tkn))

    if next == -1:
        next = len(txt)
    else:
        next += len(tkn)
    substr = txt[start:next]
    #print(f"substr found: {substr}")
    return (substr, next)

def removeComments(txt):
    cur = 0
    fin = ""
    while cur < len(txt):
        comment = False
        found = None
        if txt[cur] == '#':
            if txt[cur+1] == "#" and txt[cur+2] == "#":
                found = findAndSubstr(txt, "###", cur)
            else:
                found = findAndSubstr(txt, "\n", cur)
            comment = True
        elif txt[cur] == "\"":
            found = findAndSubstr(txt, "\"", cur, checkEsc=True)
        elif txt[cur] == "\'":
            found = findAndSubstr(txt, "\'", cur, checkEsc=True)
        else:
            fin += txt[cur]

        if found is None:
            cur += 1
        else:
            if comment:
                fin += "\n"
            else:
                fin += found[0]
            cur = found[1]
    return fin

def isIntStart(chr):
    return (chr >= '0' and chr <= '9')

def isInt(chr):
    return isIntStart(chr) or (".f_xE+-".find(chr) != -1) or (chr >= 'A' and chr <= 'F') or (chr >= 'a' and chr <= 'f')

def isIdentifier(chr):
    return (chr >= 'a' and chr <= 'z') or (chr >= 'A' and chr <= 'Z') or isIntStart(chr) or (chr == '_')

def tokenize(txt):
    cur = 0
    tokens = []
    miscTokenValues = {
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
        ',': syntax.CALIBURN_V_COMMA }
    
    for chr in "=+-*/%^&|$!~":
        miscTokenValues[chr] = syntax.CALIBURN_V_OPERATOR

    for kw in syntax.CALIBURN_KEYWORDS:
        miscTokenValues[kw] = syntax.CALIBURN_V_KEYWORD

    while cur < len(txt):
        tokenLen = 1
        token = None

        # Avoid whitespace (note to future self: use the string " \t\n\r\f\v" when implementing this in C++)
        while txt[cur].isspace(): cur += 1
        
        # Avoid comments
        if txt[cur] == "#":
            # Avoid multi-line comments
            if txt[cur+1:cur+3] == "##":
                cur += 3
                while txt[cur:cur+3] != "###":
                    cur += 1
                cur += 3
            else:
                while txt[cur] != '\n':
                    cur += 1
                # Current will end up being on a newline; need to increment once more
                cur += 1
            continue
        
        tokenID = miscTokenValues.get(txt[cur], 0)

        if tokenID:
            token = Token(txt[cur], tokenID)
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
        elif isIntStart(txt[cur]) and (isInt(txt[cur+1]) or not isIdentifier(txt[cur+1])):
            while isInt(txt[cur + tokenLen]):
                tokenLen += 1
            if isIdentifier(txt[cur + tokenLen + 1]):
                # TODO clean up all this mess, make it easier to divert this thing to getting an identifier
                pass
            token = Token(txt[cur:cur+tokenLen], syntax.CALIBURN_V_LITERAL)
        elif isIdentifier(txt[cur]):
            while isIdentifier(txt[cur + tokenLen]):
                tokenLen += 1
            tokenStr = txt[cur:cur+tokenLen]
            tokenID = syntax.CALIBURN_V_IDENTIFIER
            if miscTokenValues.get(tokenStr, 0):
                tokenID = syntax.CALIBURN_V_KEYWORD
            token = Token(tokenStr, tokenID)
        elif txt[cur] == "\"":
            while txt[cur + tokenLen] != "\"" and txt[cur + tokenLen - 1] != "\\":
                tokenLen += 1
            tokenLen += 1
            token = Token(txt[cur:cur+tokenLen], syntax.CALIBURN_V_LITERAL)
        elif txt[cur] == "\'":
            while txt[cur + tokenLen] != "\'" and txt[cur + tokenLen - 1] != "\\":
                tokenLen += 1
            tokenLen += 1
            token = Token(txt[cur:cur+tokenLen], syntax.CALIBURN_V_LITERAL)
            
        if token:
            tokens.append(token)
            cur += tokenLen
        else:
            cur += 1

    return tokens

def readFile(path):
    text = ""
    with open(path) as f:
        text = f.read()
    return text

def main(path="test.txt"):
    src = readFile(path)
    tokens = tokenize(src)
    #split = syntax.CALIBURN_WORDS_AND_SYMBOLS.findall(src)
    for tk in tokens:
        print(f"{tk.chars}, {tk.id}")

if __name__ == "__main__":
    if CALIBURN_DEBUG_MODE:
        main()
    else:
        parser = argparse.ArgumentParser(description='Prototype compiler for Caliburn')
        parser.add_argument('filepath', metavar='source', type=str,
                            help='the path or name of the source file to be compiled', required = False)

        args = parser.parse_args()
        main(args.filepath)