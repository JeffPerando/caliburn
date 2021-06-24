
def spirvOp(wordCount, opcode):
    return ((wordCount & 0xFFFF) << 16) | (opcode & 0xFFFF)

def OpNop():
    return spirvOp(1, 0)

def OpUndef():
    return spirvOp(3, 1)

def OpSizeOf():
    return spirvOp(4, 321)

def OpSourceContinued(variable):
    return spirvOp(2 + variable, 2)

def OpSource(variable):
    return spirvOp(3 + variable, 3)

def OpSourceExtension(variable):
    return spirvOp(2 + variable, 4)

def OpName(variable):
    return spirvOp(3 + variable, 5)

def OpMemberName(variable):
    return spirvOp(4 + variable, 6)

def OpString(variable):
    return spirvOp(3 + variable, 7)

def OpLine():
    return spirvOp(4, 8)

def OpNoLine():
    return spirvOp(1, 317)

def OpModuleProcessed(variable):
    return spirvOp(2 + variable, 330)