
#include "types/typevoid.h"

using namespace caliburn;

cllr::SSA RealVoid::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id == 0)
	{
		id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::TYPE_VOID));
	}

	return id;
}
