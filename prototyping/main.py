
import argparse
import re
import syntax

CALIBURN_DEBUG_MODE = True

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

def findAndSubstr(txt, tkn, start):
    next = txt.find(tkn, start + len(tkn))
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
            found = findAndSubstr(txt, "\"", cur)
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

def readFile(path):
    text = ""
    with open(path) as f:
        text = f.read()
    return text

def main(path="test.txt"):
    txt = readFile(path)
    newtxt = removeComments(txt)
    print(newtxt)
    
if __name__ == "__main__":
    if CALIBURN_DEBUG_MODE:
        main()
    else:
        parser = argparse.ArgumentParser(description='Prototype compiler for Caliburn')
        parser.add_argument('filepath', metavar='source', type=str,
                            help='the path or name of the source file to be compiled', required = False)

        args = parser.parse_args()
        main(args.filepath)