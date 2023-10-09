
#include "types/typevoid.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

sptr<cllr::LowType> RealVoid::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl == 0)
	{
		impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_VOID));
	}

	return impl;
}
