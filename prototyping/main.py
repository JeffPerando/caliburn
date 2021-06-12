
import argparse
import re

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

def readFile(path):
    text = ""
    with open(path) as f:
        text = f.read()
    return text

def main(path="test.txt"):
    txt = readFile(path)
    pat = re.compile(r"\"[^\"]+\"")
    newtxt = re.sub(pat, "", txt)
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