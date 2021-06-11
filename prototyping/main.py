
import argparse

CALIBURN_DEBUG_MODE = True

def readFile(path):
    text = ""
    with open(path) as f:
        text = f.read()
    return text

def main(path="test.txt"):
    txt = readFile(path)
    

if __name__ == "__main__":
    if CALIBURN_DEBUG_MODE:
        main()
    else:
        parser = argparse.ArgumentParser(description='Prototype compiler for Caliburn')
        parser.add_argument('filepath', metavar='source', type=str,
                            help='the path or name of the source file to be compiled')

        args = parser.parse_args()
        main(args.filepath)