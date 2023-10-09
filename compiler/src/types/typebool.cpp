
#include "types/typebool.h"

using namespace caliburn;

sptr<cllr::LowType> RealBool::emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (impl == 0)
	{
		impl = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
	}

	return impl;
}
